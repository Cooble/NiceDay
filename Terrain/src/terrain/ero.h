
// ===== Include guard for parallel/primitive selection =====
#if defined(ERO_PARALLEL_ENABLE)
#   ifdef ERO_H_PARALLEL_INCLUDED
#       define SKIP_PARALLEL 1
#   endif
#   define ERO_H_PARALLEL_INCLUDED
#else
#   ifdef ERO_H_PRIMITIVE_INCLUDED
#      define SKIP_PRIMITIVE 1
#   endif
#   define ERO_H_PRIMITIVE_INCLUDED
#endif
// skip whole file if already included
#if (defined(SKIP_PARALLEL) && defined(ERO_PARALLEL_ENABLE)) || (defined(SKIP_PRIMITIVE) && !defined(ERO_PARALLEL_ENABLE))
#else



// ===== Ero Parallel ===== (define USE_ERO_PARALLEL 1 before including to enable)
#ifdef __OPENCL_VERSION__
#undef ERO_PARALLEL_ENABLE
#define ERO_PARALLEL_ENABLE 0
#define PARALLELIZE_LOOP
#else

#if ERO_PARALLEL_ENABLE
#define PARALLELIZE_LOOP /*_Pragma("omp parallel for schedule(static)")*/
#else
#define PARALLELIZE_LOOP
#endif
#endif


// ===== Types and Macros =====
#ifdef __OPENCL_VERSION__
#define FLOAT2(x,y) ((float2)(x,y))
#define FLOAT4(x,y,z,w) ((float4)(x,y,z,w))
#define FMAX(a,b) fmax(a,b)
#define FABS(x) fabs(x)
#define FMIN(a,b) fmin(a,b)
#else
#include <glm/glm.hpp>
#define FLOAT2(x,y) glm::vec2((x),(y))
#define FLOAT4(x,y,z,w) glm::vec4((x),(y),(z),(w))
#define FMAX(a,b) ::fmax(a,b)
#define FABS(x) ::fabs(x)
#define FMIN(a,b) ::fmin(a,b)
#endif

// ===== Close into Parallel namespace =====
#if ERO_PARALLEL_ENABLE
namespace EroParallel
{

#endif


// ===== Constants =====
#ifdef __OPENCL_VERSION__
#define constant __constant
#else
#define constant inline
#endif

constant float pPipeArea = 0.6f;
constant float pGravity = 9.81f;
constant float pPipeLen = 1.f;
constant float pLL = 1.f;
constant float pMaxDissolve = 0.1f;


#ifdef __OPENCL_VERSION__
#define GET_X_Y int x = get_global_id(0);\
                 int y = get_global_id(1);\
                 int idx = y * w + x;
#define SIGN(x) sign(x)

#else
#define __kernel inline
#define __global
#define GET_X_Y \
	PARALLELIZE_LOOP \
	for (int y = 1; y < h-1; y++)\
	for (int x = 1, idx = y*w + 1; x < w - 1; x++, idx++)





#include "EulerSim.h"
using float4 = glm::vec4;
using float2 = glm::vec2;

inline float4 fmax(float4 a, float4 b)
{
	return glm::max(a, b);
}

inline float2 fmax(float2 a, float2 b)
{
	return glm::max(a, b);
}

inline float4 fabs(float4 a)
{
	return glm::abs(a);
}

inline float clamp(float x, float minVal, float maxVal)
{
	return glm::clamp(x, minVal, maxVal);
}

inline int clamp(int x, int minVal, int maxVal)
{
	return glm::clamp(x, minVal, maxVal);
}

#define SIGN(x) glm::sign(x)

#endif

// Bilinear interpolation for 2D data
inline float interpolate2D(__global const float* data, int width, int height, float x, float y)
{
	int x0 = (int)x;
	int y0 = (int)y;
	int x1 = x0 + 1;
	int y1 = y0 + 1;
	x0 = clamp(x0, 0, width - 1);
	y0 = clamp(y0, 0, height - 1);
	x1 = clamp(x1, 0, width - 1);
	y1 = clamp(y1, 0, height - 1);
	float xLerp = x - x0;
	float yLerp = y - y0;
	float h00 = data[y0 * width + x0];
	float h01 = data[y0 * width + x1];
	float h10 = data[y1 * width + x0];
	float h11 = data[y1 * width + x1];
	return h00 * (1 - xLerp) * (1 - yLerp) +
		h01 * (xLerp) * (1 - yLerp) +
		h10 * (yLerp) * (1 - xLerp) +
		h11 * (xLerp) * (yLerp);
}


// 1. Rain & Evaporation (Merged)
__kernel void ero1_kernel(
	__global float* water_height,
	const int w,
	const int h,
	const float K_rain,
	const float K_evaporation,
	const float K_dt)
{
	GET_X_Y
	{
		float wh = water_height[idx];

		// Apply Rain
		wh += K_dt * K_rain;

		// Apply Evaporation
		wh *= (1.0f - K_evaporation * K_dt);

		// Remove incredibly small values
		float evaporationEpsilon = 0.001f;
		if (wh < evaporationEpsilon)
			wh = 0.0f;

		water_height[idx] = wh;
	}
}

#ifndef __OPENCL_VERSION__
inline void ero1(EulerGround& g, Euler::EulerSettings* s)
{
	if (!s->e_rain)
		return;

	ero1_kernel(
		g.water_height.data(),
		g.width,
		g.height,
		s->K_rain,
		s->K_evaporation,
		s->K_dt);
}
#endif


inline void ero2_borders(int idx, int x, int y, int w, int h, __global float4* flux)
{
	float4 f = flux[idx];

	// Left/right border flux zeroing
	if (x == 1)
		f.x = 0.0f; // left outflux
	else if (x == w - 2)
		f.y = 0.0f; // right outflux

	// Top/bottom border flux zeroing
	if (y == 1)
		f.z = 0.0f; // top outflux
	else if (y == h - 2)
		f.w = 0.0f; // bottom outflux

	flux[idx] = f;
}

// 2. Flux computation
__kernel void ero2_kernel(
	__global const float* terrain_height,
	__global const float* water_height,
	__global float4* flux,
	const int w,
	const int h,
	const float K_dt)
{
	GET_X_Y
	{
		// Compute total height
		float totalHeight_c = water_height[idx] + terrain_height[idx];
		float totalHeight_l = water_height[idx - 1] + terrain_height[idx - 1];
		float totalHeight_r = water_height[idx + 1] + terrain_height[idx + 1];
		float totalHeight_t = water_height[idx - w] + terrain_height[idx - w];
		float totalHeight_b = water_height[idx + w] + terrain_height[idx + w];

		float4 deltaH = FLOAT4(
			totalHeight_c - totalHeight_l,
			totalHeight_c - totalHeight_r,
			totalHeight_c - totalHeight_t,
			totalHeight_c - totalHeight_b
		);

		float fluxFactor = K_dt * pPipeArea / pPipeLen * pGravity;
		flux[idx] = fmax(FLOAT4(0.f, 0.f, 0.f, 0.f), flux[idx] + deltaH * fluxFactor);

		float sumF = flux[idx].x + flux[idx].y + flux[idx].z + flux[idx].w;

		if (sumF > 0)
		{
			float waterVolume = water_height[idx] * pLL * pLL;
			float outVolume = sumF * K_dt;
			float adjustmentFactor = fmin(1.f, waterVolume / outVolume);

			flux[idx] *= adjustmentFactor;
		}
		ero2_borders(idx, x, y, w, h, flux);
	}
}
#ifndef __OPENCL_VERSION__
inline void ero2(EulerGround& g, Euler::EulerSettings* s)
{
	if (!s->e_flow)
		return;

	ero2_kernel(g.terrain_height.data(),
	            g.water_height.data(),
	            g.flux.data(),
	            g.width,
	            g.height,
	            s->K_dt);
}
#endif


inline void ero3_borders(int idx, int x, int y, int w, int h, float2* velocity)
{
	float2 v = velocity[idx];

	// left/right border nulling
	if (x == 1)
		v.x = FMAX(0.0f, v.x);
	else if (x == w - 2)
		v.x = fmin(0.0f, v.x);

	// top/bottom border nulling
	if (y == 1)
		v.y = FMAX(0.0f, v.y);
	else if (y == h - 2)
		v.y = fmin(0.0f, v.y);

	velocity[idx] = v;
}

// 3. Water height update and velocity computation
__kernel void ero3_kernel(
	__global float* water_height,
	__global const float4* flux,
	__global float2* velocity,
	const int w,
	const int h,
	const float K_dt)
{
	GET_X_Y
	{
		float sumIn =
			flux[y * w + x - 1].y +
			flux[y * w + x + 1].x +
			flux[(y - 1) * w + x].w +
			flux[(y + 1) * w + x].z;
		float sumOut = flux[idx].x + flux[idx].y + flux[idx].z + flux[idx].w;

		float deltaV = (sumIn - sumOut) * K_dt;
		float deltaH = deltaV / (pLL * pLL);
		water_height[idx] = FMAX(0.f, water_height[idx] + deltaH);
		float meanH = water_height[idx] - deltaH / 2.f;

		if (meanH > 0)
		{
			float fluxX =
				flux[y * w + x - 1].y -
				flux[idx].x +
				flux[idx].y -
				flux[y * w + x + 1].x;
			float fluxY =
				flux[(y - 1) * w + x].w -
				flux[idx].z +
				flux[idx].w -
				flux[(y + 1) * w + x].z;
			velocity[idx] = FLOAT2(fluxX, fluxY) / (meanH * pLL);
		}
		else
		{
			velocity[idx] = FLOAT2(0.f, 0.f);
		}


		ero3_borders(idx, x, y, w, h, velocity);
	}
}
#ifndef __OPENCL_VERSION__
inline void ero3(EulerGround& g, Euler::EulerSettings* s)
{
	if (!s->e_flow)
		return;

	ero3_kernel(g.water_height.data(),
	            g.flux.data(),
	            g.velocity.data(),
	            g.width,
	            g.height,
	            s->K_dt);
}
#endif


// 4. Erosion and deposition
__kernel void ero4_kernel(
	__global const float* terrain_height,
	__global float* new_terrain_height,
	__global const float* water_height,
	__global float* sediment,
	__global const float2* velocity,
	__global const float* perlin_map,
	__global const float* original_height,
	const int w,
	const int h,
	const float K_sediment_capacity,
	const float K_tilt_minimum,
	const float K_s_dissolving,
	const float K_d_depositing)
{
	float erosionClamp = 10.f;

	GET_X_Y
	{
		float gradX = (terrain_height[idx + 1] - terrain_height[idx - 1]) / 2;
		float gradY = (terrain_height[idx + w] - terrain_height[idx - w]) / 2;

		float grade = clamp(gradX * gradX + gradY * gradY, -erosionClamp, erosionClamp);
		float sin_local_tilt = sqrt(grade / (1 + grade));

		sin_local_tilt = FMAX(sin_local_tilt, K_tilt_minimum);

		float vel_length = sqrt(velocity[idx].x * velocity[idx].x + velocity[idx].y * velocity[idx].y);
		float capacity = K_sediment_capacity * vel_length * sin_local_tilt * fmin(1.f, water_height[idx]);
		float perlinFactor = perlin_map[idx];

		// The deeper the harder to dissolve
		float depthFactor = 1.f / (1.f + original_height[idx] - terrain_height[idx]);
		if (original_height[idx] - terrain_height[idx] < 0)
			depthFactor = 1.f;
		if (capacity > sediment[idx])
		{
			float dSoil = K_s_dissolving * (capacity - sediment[idx]) * perlinFactor * depthFactor;

			// Limit dissolve
			dSoil = fmin(dSoil, pMaxDissolve);

			// Limit dissolve to the terrain height
			dSoil = fmin(dSoil, terrain_height[idx]);


			new_terrain_height[idx] = terrain_height[idx] - dSoil;
			sediment[idx] = sediment[idx] + dSoil;
		}
		else
		{
			float dSoil = K_d_depositing * (sediment[idx] - capacity);

			// Limit deposit
			dSoil = fmin(dSoil, pMaxDissolve);

			new_terrain_height[idx] = terrain_height[idx] + dSoil;
			sediment[idx] = sediment[idx] - dSoil;
		}
	}
}
#ifndef __OPENCL_VERSION__
inline void ero4(EulerGround& g, Euler::EulerSettings* s)
{
	if (!s->e_erosion)
		return;

	ero4_kernel(
		g.terrain_height.data(),
		g.new_terrain_height.data(),
		g.water_height.data(),
		g.sediment.data(),
		g.velocity.data(),
		g.perlin_map.data(),
		g.original_height.data(),
		g.width,
		g.height,
		s->K_sediment_capacity,
		s->K_tilt_minimum,
		s->K_s_dissolving,
		s->K_d_depositing);

	std::swap(g.new_terrain_height, g.terrain_height);
}
#endif


// 5. Sediment transport
__kernel void ero5_kernel(
	__global const float* sediment,
	__global float* new_sediment,
	__global const float2* velocity,
	const int w,
	const int h,
	const float K_dt)
{
	GET_X_Y
	{
		float velx = velocity[idx].x;
		float vely = velocity[idx].y;

		float fx = (float)x - velx * K_dt;
		float fy = (float)y - vely * K_dt;

		new_sediment[idx] = interpolate2D(sediment, w, h, fx, fy);
	}
}
#ifndef __OPENCL_VERSION__
inline void ero5(EulerGround& g, Euler::EulerSettings* s)
{
	if (!s->e_erosion)
		return;
	ero5_kernel(
		g.sediment.data(),
		g.new_sediment.data(),
		g.velocity.data(),
		g.width,
		g.height,
		s->K_dt);

	std::swap(g.sediment, g.new_sediment);
}
#endif


// 7. Landslide - single pass kernel that processes a 2x2 block
__kernel void ero7_kernel(
	__global float* terrain_height,
	const int w,
	const int h,
	const int offset_x,
	const int offset_y,
	const float K_landSlideSpeed,
	const float K_landSlideCutoffAngle,
	const float K_dt)
{
#ifdef __OPENCL_VERSION__
	int x = get_global_id(0) * 2 + 1 + offset_x;
	int y = get_global_id(1) * 2 + 1 + offset_y;
#else
	for (int y = 1 + offset_y; y < h - 1; y += 2)
		for (int x = 1 + offset_x; x < w - 1; x += 2)
#endif
		{
			// Check bounds
			if (x >= w - 1 || y >= h - 1)
#ifdef __OPENCL_VERSION__
			return;
#else
				continue;
#endif

			int idx = y * w + x;

			// Process 2x2 block: current pixel and its right and bottom neighbors
			float height = terrain_height[idx];
			float heightR = terrain_height[idx + 1];
			float heightB = terrain_height[idx + w];

			float deltaR = heightR - height;
			float deltaB = heightB - height;

			float takeR = deltaR * K_landSlideSpeed * K_dt;
			float takeB = deltaB * K_landSlideSpeed * K_dt;

			float signR = SIGN(takeR);
			float signB = SIGN(takeB);

			takeR = FMAX(0.f, FABS(takeR) - K_landSlideCutoffAngle) * signR;
			takeB = FMAX(0.f, FABS(takeB) - K_landSlideCutoffAngle) * signB;
			// should produce something like
			//                   /
			//                  /
			//   -------+-------
			//  /
			// /

			terrain_height[idx] += takeR + takeB;
			terrain_height[idx + 1] -= takeR;
			terrain_height[idx + w] -= takeB;
		}
}


#ifndef __OPENCL_VERSION__
#define USE_OPTIMIZED_CPU_ERO7

// Optimized CPU version of ero7 that processes entire terrain in a single pass, no race conditions
inline void ero7_cpu(
	float* terrain_height,
	const int w,
	const int h,
	const float K_landSlideSpeed,
	const float K_landSlideCutoffAngle,
	const float K_dt)
{
	for (int y = 1; y < h - 2; y++)
		for (int x = 1; x < w - 2; x++)
		{
			auto idx = y * w + x;

			// Process 2x2 block: current pixel and its right and bottom neighbors
			float height = terrain_height[idx];
			float heightR = terrain_height[idx + 1];
			float heightB = terrain_height[idx + w];

			float deltaR = heightR - height;
			float deltaB = heightB - height;

			float takeR = deltaR * K_landSlideSpeed * K_dt;
			float takeB = deltaB * K_landSlideSpeed * K_dt;

			float signR = SIGN(takeR);
			float signB = SIGN(takeB);

			takeR = FMAX(0.f, FABS(takeR) - K_landSlideCutoffAngle) * signR;
			takeB = FMAX(0.f, FABS(takeB) - K_landSlideCutoffAngle) * signB;
			// should produce something like
			//                   /
			//                  /
			//   -------+-------
			//  /
			// /

			terrain_height[idx] += takeR + takeB;
			terrain_height[idx + 1] -= takeR;
			terrain_height[idx + w] -= takeB;
		}
}

inline void ero7(EulerGround& g, Euler::EulerSettings* s)
{
	if (!s->e_landslide)
		return;

#ifdef USE_OPTIMIZED_CPU_ERO7
	ero7_cpu(g.terrain_height.data(), g.width, g.height,
	         s->K_landSlideSpeed, s->K_landSlideCutoffAngle, s->K_dt);
	return;
#endif

	// 4 passes with different offsets to cover entire terrain
	// Pass 1: (0, 0) - process blocks starting at even x, even y
	ero7_kernel(g.terrain_height.data(), g.width, g.height, 0, 0,
	            s->K_landSlideSpeed, s->K_landSlideCutoffAngle, s->K_dt);

	// Pass 2: (1, 0) - process blocks starting at odd x, even y
	ero7_kernel(g.terrain_height.data(), g.width, g.height, 1, 0,
	            s->K_landSlideSpeed, s->K_landSlideCutoffAngle, s->K_dt);

	// Pass 3: (0, 1) - process blocks starting at even x, odd y
	ero7_kernel(g.terrain_height.data(), g.width, g.height, 0, 1,
	            s->K_landSlideSpeed, s->K_landSlideCutoffAngle, s->K_dt);

	// Pass 4: (1, 1) - process blocks starting at odd x, odd y
	ero7_kernel(g.terrain_height.data(), g.width, g.height, 1, 1,
	            s->K_landSlideSpeed, s->K_landSlideCutoffAngle, s->K_dt);
}
#endif


#if ERO_PARALLEL_ENABLE
} // namespace EroParallel
#endif

#endif // include guard