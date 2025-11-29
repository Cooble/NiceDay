#include "ndpch.h"
#include "EulerSim.h"
#include <ImGuiFileDialog.h>

#include "TerrainLayer.h"
#include "TUtils.h"
#include "core/NBT.h"

#include <immintrin.h>

#include <algorithm>

#include "ero.h"
#define ERO_PARALLEL_ENABLE 1
#include "ero.h"


#include "ero_simd.h"
#define EROSIMD_PARALLEL_ENABLE 1
#include "ero_simd.h"

#include "cl_context.h"

#undef ND_PROFILE_METHOD
#define ND_PROFILE_METHOD()


//#define USE_PARALLEL


#ifdef USE_PARALLEL
#define PARALLELIZE_LOOP _Pragma("omp parallel for schedule(static)")
#else
#define PARALLELIZE_LOOP
#endif


#ifdef USE_PARALLEL
#define PARALLELIZE_LOOP_SIMD _Pragma("omp parallel for schedule(static)")
#else
#define PARALLELIZE_LOOP_SIMD
#endif



#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <algorithm>


using namespace nd;

void Euler::init(EulerGround& g, EulerSettings* s,bool generatePerlin)
{
	this->s = s;
	perlinMap.resize(g.width * g.width);
	sedimentDontUse = g.sediment;

	g.new_sediment = g.sediment;
	g.original_height = g.terrain_height;
	originalHeight = g.terrain_height;
	g.new_terrain_height = g.terrain_height;
	g.total_height = g.terrain_height;

	if (generatePerlin)
		ter::generate2DPerlin(REINTERPRET_AS(std::vector<gfloat>, perlinMap), g.width, g.height);
	g.perlin_map = perlinMap;

	ZeroMemory(g.water_height.data(), g.water_height.size() * sizeof(decltype(g.water_height)::value_type));

	if (sim_type == OPENCL) 
		cl->init(g, *s);
	refreshParams(g, *s);

}

void Euler::refreshParams(EulerGround& g,EulerSettings& s)
{
	if (sim_type == OPENCL && settings_dirty) {
	//if (sim_type == OPENCL) {
		cl->upload_params(g, s);
		settings_dirty = false;
	}
}

static uint64_t sMeasureFunctionTime = 1;

void measureFunction(std::function<void(EulerGround&,Euler::EulerSettings*)> a1, std::function<void(EulerGround&, Euler::EulerSettings*)> a2, EulerGround& g,Euler::EulerSettings* s)
{
	uint64_t micros1, micros2;
	{
		TimerStaper p("");
		for (int i = 0; i < sMeasureFunctionTime; ++i)
			a1(g,s);
		micros1 = p.getUS();
	}
	{
		TimerStaper p("");
		for (int i = 0; i < sMeasureFunctionTime; ++i)
			a2(g,s);
		micros2 = p.getUS();
	}
	ND_BUG("SIMD took {} us, BASIC took {} us, SpeedUp of {}", micros1 / sMeasureFunctionTime,
	       micros2 / sMeasureFunctionTime, (gfloat)micros2 / (gfloat)micros1);
}

#define measureFuncFixed(name) measureFunction(&name##_simd, &(name), g,s )

void Euler::step(EulerGround& g)
{
	if (g.height * g.height != sedimentDontUse.size())
		init(g,this->s);

	switch (sim_type)
	{
	case CPU_BASIC:
		ero1(g, s);
		ero2(g, s);
		ero3(g, s);
		ero4(g, s);
		ero5(g, s);
		ero6(g, s);
		ero7(g, s);

		//ero1_old(g);
		//ero2_old(g);
		//ero3_old(g);
		//ero4_old(g);
		//ero5_old(g);
		//ero6_old(g);
		//ero7_old(g);

		return;

	case CPU_PARALLEL:
		EroParallel::ero1(g, s);
		EroParallel::ero2(g, s);
		EroParallel::ero3(g, s);
		EroParallel::ero4(g, s);
		EroParallel::ero5(g, s);
		EroParallel::ero6(g, s);
		EroParallel::ero7(g, s);
		return;

	case CPU_SIMD:
		ero1_simd(g, s);
		ero2_simd(g, s);
		//ero2_simd_fix_borders(g);
		ero3_simd(g, s);
		//ero3_simd_fix_borders(g);
		ero4_simd(g, s);
		ero5_simd(g, s);
		ero6_simd(g, s);
		ero7_simd(g, s);

		//ero1_simd_old(g);
		//ero2_simd_old(g);
		//ero3_simd_old(g);
		//ero4_simd_old(g);
		//ero5_simd_old(g);
		//ero6_simd_old(g);
		//ero7_simd_old(g);

		return;
	case SIMD_PARALLEL:
		EroParallel::ero1_simd(g, s);
		EroParallel::ero2_simd(g, s);
		EroParallel::ero3_simd(g, s);
		EroParallel::ero4_simd(g, s);
		EroParallel::ero5_simd(g, s);
		EroParallel::ero6_simd(g, s);
		EroParallel::ero7_simd(g, s);
		return;
	case OPENCL:
		//cl->upload_all(g);
		//cl->upload_params(g, *s);
		cl->step(*s);
		return;
	}


	static bool first = true;
	if (first)
	{
		first = false;
		sMeasureFunctionTime = 1; //warmup
	}
	else
		sMeasureFunctionTime = 50;


	ND_BUG("======================Measuring Euler Steps");
	measureFuncFixed(ero1);
	measureFuncFixed(ero2);
	measureFuncFixed(ero3);
	measureFuncFixed(ero4);
	measureFuncFixed(ero5);
	measureFuncFixed(ero6);
	measureFuncFixed(ero7);
	ND_BUG("======================Done");
}

void Euler::stepRender(EulerGround& g)
{
	if (sim_type == OPENCL)
		cl->download_graphics(g);
}

void Euler::imguiRender()
{
	EulerSettings pastSettings = *s;

	using namespace ter;
	ImGui::Checkbox("Rain", &s->e_rain);
	ImGui::Checkbox("Flow", &s->e_flow);
	ImGui::Checkbox("Erosion", &s->e_erosion);
	ImGui::Checkbox("Evaporation", &s->e_evaporation);
	ImGui::Checkbox("Landslide", &s->e_landslide);

	ImGui::SeparatorText("Euler Settings");
	ImGui::PushID(this);


	InputGFloat("Gravity", &s->K_g, 0, 0, "%.6f");
	SliderGFloat("Dt", &s->K_dt, 0, (gfloat)0.1, "%.6f");
	SliderGFloat("Rain", &s->K_rain, 0, 10, "%.3f");
	SliderGFloat("Sediment Capacity", &s->K_sediment_capacity, 0, (gfloat)0.5, "%.6f");
	SliderGFloat("Dissolving", &s->K_s_dissolving, 0, (gfloat)0.5, "%.6f");
	SliderGFloat("Depositing", &s->K_d_depositing, 0, (gfloat)0.5, "%.6f");
	SliderGFloat("Evaporation", &s->K_evaporation, 0, (gfloat)0.5, "%.6f");

	SliderGFloat("Tilt Minimum", &s->K_tilt_minimum, 0, 10, "%.3f");
	SliderGFloat("Land Slide Speed", &s->K_landSlideSpeed, 0, 1, "%.3f");
	SliderGFloat("Land Slide Cutoff Angle", &s->K_landSlideCutoffAngle, 0, 1, "%.3f");

	ImGui::PopID();


	// ======== CONFIG SERIALIZATION
	{
		if (ImGui::Button("Load Config"))
		{
			IGFD::FileDialogConfig config;
			config.path = ".";
			ImGuiFileDialog::Instance()->OpenDialog("LoadConfig1", "Choose config file to open", ".json", config);
		}
		ImGui::SetItemTooltip("Load simulation parameters from file\n"
			"Config is a json file with all the parameters of the euler simulation.");
		ImGui::SameLine();
		if (ImGui::Button("Save Config"))
		{
			IGFD::FileDialogConfig config;
			config.path = ".";
			ImGuiFileDialog::Instance()->OpenDialog("SaveConfig1", "Choose config file to save", ".json", config);
		}
		ImGui::SetItemTooltip("Save simulation parameters to file\n"
			"Config is a json file with all the parameters of the euler simulation.");

		if (ImGuiFileDialog::Instance()->Display("LoadConfig1"))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				ND_BUG("Loading config from {}", filePathName);
				nd::NBT nbt;
				nd::NBT::loadFromFile(filePathName, nbt);
				load(nbt);
			}
			ImGuiFileDialog::Instance()->Close();
		}
		if (ImGuiFileDialog::Instance()->Display("SaveConfig1"))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				ND_BUG("Saving config to {}", filePathName);
				nd::NBT nbt;
				save(nbt);
				nd::NBT::saveToFile(filePathName, nbt);
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}

	// check if change happened
	if (!(pastSettings==*s))
		settings_dirty = true;
}

void Euler::save(nd::NBT& src)
{
	NBT_SAVE(src, s->K_dt);
	NBT_SAVE(src, s->K_rain);
	NBT_SAVE(src, s->K_g);
	NBT_SAVE(src, s->K_sediment_capacity);
	NBT_SAVE(src, s->K_s_dissolving);
	NBT_SAVE(src, s->K_d_depositing);
	NBT_SAVE(src, s->K_evaporation);
	NBT_SAVE(src, s->K_tilt_minimum);
	NBT_SAVE(src, s->K_landSlideSpeed);
	NBT_SAVE(src, s->K_landSlideCutoffAngle);
}

void Euler::load(nd::NBT& src)
{
	NBT_LOAD(src, s->K_dt);
	NBT_LOAD(src, s->K_rain);
	NBT_LOAD(src, s->K_g);
	NBT_LOAD(src, s->K_sediment_capacity);
	NBT_LOAD(src, s->K_s_dissolving);
	NBT_LOAD(src, s->K_d_depositing);
	NBT_LOAD(src, s->K_evaporation);
	NBT_LOAD(src, s->K_tilt_minimum);
	NBT_LOAD(src, s->K_landSlideSpeed);
	NBT_LOAD(src, s->K_landSlideCutoffAngle);
}

Euler::Euler() :cl(new EroCLContext())
{
}

Euler::~Euler()
{
	delete cl;
}

void Euler::ero1_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	if (!s->e_rain)
		return;

	// 1. rain
	PARALLELIZE_LOOP
	for (int y = 1; y < h - 1; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto d = glm::ivec2(x, y) - glm::ivec2(w / 2);

			gfloat increase = (d.x * d.x + d.y * d.y < 100 || (x > w / 2 - 10 && x < w / 2 + 10))
				                  ? s->K_rain
				                  : 0;

			// rain everywhere same
			increase = s->K_rain;
			g.water_height[x + y * w] += s->K_dt * increase;
		}
}

void Euler::ero2_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;


	static std::vector<gfloat> totalHeight(g.terrain_height.size());
	if (totalHeight.size() != g.terrain_height.size())
		totalHeight.resize(g.terrain_height.size());

	PARALLELIZE_LOOP
	for (int i = 0; i < totalHeight.size(); i++)
		totalHeight[i] = g.water_height[i] + g.terrain_height[i];


	if (!s->e_flow)
		return;

	// 2. flux
	PARALLELIZE_LOOP
	for (int y = 1; y < h - 1; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;
			auto deltaH = gvec4(
				totalHeight[idx] - totalHeight[y * w + x - 1],
				totalHeight[idx] - totalHeight[y * w + x + 1],
				totalHeight[idx] - totalHeight[(y - 1) * w + x],
				totalHeight[idx] - totalHeight[(y + 1) * w + x]);

			auto fluxFactor = s->K_dt * pPipeArea / pPipeLen * pGravity;
			g.flux[idx] = glm::max(gvec4(0.f), g.flux[idx] + deltaH * fluxFactor);

			//auto sumF = glm::compAdd(g.flux[idx]);
			auto sumF = g.flux[idx].x + g.flux[idx].y + g.flux[idx].z + g.flux[idx].w;

			if (sumF > 0)
			{
				auto waterVolume = g.water_height[idx] * pLL * pLL;
				auto outVolume = sumF * s->K_dt;
				auto adjustmentFactor = glm::min((gfloat)1, waterVolume / outVolume);

				g.flux[idx] *= adjustmentFactor;
			}
		}

	ero2_fix_borders(g);
}

void Euler::ero3_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	if (!s->e_flow)
		return;

	// 3. water height
	PARALLELIZE_LOOP
	for (int y = 1; y < h - 1; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;
			auto sumIn =
				+g.flux[y * w + x - 1].y
				+ g.flux[y * w + x + 1].x
				+ g.flux[(y - 1) * w + x].w
				+ g.flux[(y + 1) * w + x].z;
			auto sumOut = g.flux[idx].x + g.flux[idx].y + g.flux[idx].z + g.flux[idx].w;

			auto deltaV = (sumIn - sumOut) * s->K_dt;
			auto deltaH = deltaV / (pLL * pLL);
			g.water_height[idx] = glm::max((gfloat)0.f, g.water_height[idx] + deltaH);
			auto meanH = g.water_height[idx] - deltaH / 2.f;

			if (meanH > 0)
			{
				auto fluxX =
					+g.flux[y * w + x - 1].y
					- g.flux[idx].x
					+ g.flux[idx].y
					- g.flux[y * w + x + 1].x;
				auto fluxY =
					+g.flux[(y - 1) * w + x].w
					- g.flux[idx].z
					+ g.flux[idx].w
					- g.flux[(y + 1) * w + x].z;
				g.velocity[idx] = gvec2(fluxX, fluxY) / (meanH * pLL);
			}
			else
				g.velocity[idx] = gvec2(0.f);
		}
	ero3_fix_borders(g);
}

void Euler::ero4_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	// 4. erosion
	constexpr gfloat erosionClamp = 10;

	if (!s->e_erosion)
		return;

	PARALLELIZE_LOOP
	for (int y = 1; y < h - 1; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;

			auto gradX = (g.terrain_height[y * w + x + 1] - g.terrain_height[y * w + x - 1]) / 2;
			auto gradY = (g.terrain_height[(y + 1) * w + x] - g.terrain_height[(y - 1) * w + x]) / 2;

			auto grade = glm::clamp(gradX * gradX + gradY * gradY, -erosionClamp, erosionClamp);
			auto sin_local_tilt = glm::sqrt(grade / (1 + grade));

			sin_local_tilt = glm::max(sin_local_tilt, s->K_tilt_minimum);

			auto capacity = s->K_sediment_capacity * glm::length(g.velocity[idx]) * sin_local_tilt * glm::min(
				(gfloat)1, g.water_height[idx]);


			auto perlinFactor = perlinMap[idx];

			// the deeper the harder to dissolve
			auto depthFactor = 1 / (1 + originalHeight[idx] - g.terrain_height[idx]);
			if (originalHeight[idx] - g.terrain_height[idx] < 0)
				depthFactor = 1;

			if (capacity > g.sediment[idx])
			{
				auto dSoil = s->K_s_dissolving * (capacity - g.sediment[idx]) * perlinFactor * depthFactor;

				// limit dissolve
				dSoil = glm::min(dSoil, pMaxDissolve);

				// limit dissolve to the terrain height
				dSoil = glm::min(dSoil, g.terrain_height[idx]);

				g.terrain_height[idx] = g.terrain_height[idx] - dSoil;
				g.sediment[idx] = g.sediment[idx] + dSoil;
			}
			else
			{
				auto dSoil = s->K_d_depositing * (g.sediment[idx] - capacity);

				// limit deposit
				dSoil = glm::min(dSoil, pMaxDissolve);

				g.terrain_height[idx] = g.terrain_height[idx] + dSoil;
				g.sediment[idx] = g.sediment[idx] - dSoil;
			}
		}
}

void Euler::ero5_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	if (!s->e_erosion)
		return;

	// 5. sediment transport
	PARALLELIZE_LOOP
	for (int y = 1; y < h - 1; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;

			gfloat velx = g.velocity[idx].x;
			gfloat vely = g.velocity[idx].y;

			gfloat fx = (gfloat)x - velx * s->K_dt;
			gfloat fy = (gfloat)y - vely * s->K_dt;


			g.new_sediment[idx] = ter::interpolate2D(REINTERPRET_AS(std::vector<gfloat>, g.sediment), w, h, fx, fy);
			//g.sediment[idx] = g.sediment[idx];
		}
	std::swap(g.new_sediment, g.sediment);
}

void Euler::ero6_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	if (!s->e_evaporation)
		return;


	// 6. evaporation
	PARALLELIZE_LOOP
	for (int y = 1; y < h - 1; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;
			g.water_height[idx] *= 1 - s->K_evaporation * s->K_dt;

			// remove the incredibly small values
			constexpr gfloat evaporationEpsilon = 0.001;
			if (g.water_height[idx] < evaporationEpsilon)
				g.water_height[idx] = 0;
		}
}

void Euler::ero7_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	if (!s->e_landslide)
		return;

	// 7. landslide
	PARALLELIZE_LOOP
	for (int y = 1; y < h - 1; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;

			auto& height = g.terrain_height[idx];

			auto heightN = gvec2(
				g.terrain_height[idx + 1],
				g.terrain_height[idx + w]);

			auto delta = heightN - height;

			auto takeN = s->K_landSlideSpeed * s->K_dt * delta;

			auto signs = glm::sign(takeN);
			takeN = glm::max(glm::abs(takeN) - s->K_landSlideCutoffAngle, (gfloat)0.f) * signs;
			// should produce something like
			//                   /
			//                  /
			//   -------+-------
			//  /
			// /

			height += glm::compAdd(takeN);
			g.terrain_height[idx + 1] -= takeN.x;
			g.terrain_height[idx + w] -= takeN.y;
		}
}


void Euler::ero1_simd_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	const float val = s->K_dt * s->K_rain;

	__m512 vval = _mm512_set1_ps(val);

	PARALLELIZE_LOOP_SIMD
	for (int y = 1; y < h - 1; y++)
	{
		int offset = y * w;

		for (int x = 1; x < w - 16; x += 16)
		{
			__m512 vdata = _mm512_loadu_ps(&g.water_height[offset + x]);
			vdata = _mm512_add_ps(vdata, vval);
			_mm512_storeu_ps(&g.water_height[offset + x], vdata);
		}
	}
}

void Euler::ero2_simd_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	static std::vector<gfloat> totalHeight(g.terrain_height.size());
	if (totalHeight.size() != g.terrain_height.size())
		totalHeight.resize(g.terrain_height.size());

	// totalHeight = terrain + water
	PARALLELIZE_LOOP_SIMD
	for (int y = 0; y < h; y++)
	{
		int offset = y * w;
		for (int x = 0; x < w; x += 16)
		{
			__m512 vdataA = _mm512_loadu_ps(&g.terrain_height[offset + x]);
			__m512 vdataB = _mm512_loadu_ps(&g.water_height[offset + x]);
			vdataA = _mm512_add_ps(vdataA, vdataB);
			_mm512_storeu_ps(&totalHeight[offset + x], vdataA);
		}
	}
	if (!s->e_flow) return;

	const float fluxFactor = s->K_dt * pPipeArea / pPipeLen * pGravity;
	const __m512 fluxFactorSIMD = _mm512_set1_ps(fluxFactor);


	for (int y = 1; y < h - 1; ++y)
	{
		int rowC = y * w;

		//for (int x = 1; x < w - 1; ++x)
		PARALLELIZE_LOOP_SIMD
		for (int x = 1; x < w - 16; x += 16)
		{
			int idx = rowC + x;
			int idxL = idx - 1;
			int idxR = idx + 1;
			int idxU = idx - w;
			int idxD = idx + w;


			// height load
			__m512 cur = _mm512_loadu_ps(&totalHeight[idx]);
			__m512 left = _mm512_loadu_ps(&totalHeight[idxL]);
			__m512 right = _mm512_loadu_ps(&totalHeight[idxR]);
			__m512 up = _mm512_loadu_ps(&totalHeight[idxU]);
			__m512 down = _mm512_loadu_ps(&totalHeight[idxD]);

			// height delta
			__m512 deltaH_x1 = _mm512_sub_ps(cur, left);
			__m512 deltaH_x2 = _mm512_sub_ps(cur, right);
			__m512 deltaH_y1 = _mm512_sub_ps(cur, up);
			__m512 deltaH_y2 = _mm512_sub_ps(cur, down);

			// multiply by flux factor
			deltaH_x1 = _mm512_mul_ps(deltaH_x1, fluxFactorSIMD);
			deltaH_x2 = _mm512_mul_ps(deltaH_x2, fluxFactorSIMD);
			deltaH_y1 = _mm512_mul_ps(deltaH_y1, fluxFactorSIMD);
			deltaH_y2 = _mm512_mul_ps(deltaH_y2, fluxFactorSIMD);

			// flux current
			__m512 curFluxL = _mm512_loadu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 0);
			__m512 curFluxR = _mm512_loadu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 1);
			__m512 curFluxU = _mm512_loadu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 2);
			__m512 curFluxD = _mm512_loadu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 3);

			// add delta to flux
			curFluxL = _mm512_add_ps(curFluxL, deltaH_x1);
			curFluxR = _mm512_add_ps(curFluxR, deltaH_x2);
			curFluxU = _mm512_add_ps(curFluxU, deltaH_y1);
			curFluxD = _mm512_add_ps(curFluxD, deltaH_y2);

			// clamp to >=0
			__m512 zero = _mm512_setzero_ps();
			curFluxL = _mm512_max_ps(curFluxL, zero);
			curFluxR = _mm512_max_ps(curFluxR, zero);
			curFluxU = _mm512_max_ps(curFluxU, zero);
			curFluxD = _mm512_max_ps(curFluxD, zero);

			// total flux outflow
			__m512 sum = _mm512_add_ps(curFluxL, _mm512_add_ps(curFluxR, _mm512_add_ps(curFluxU, curFluxD)));

			// mask for sum > 0
			__mmask16 mask = _mm512_cmp_ps_mask(sum, zero, _CMP_GT_OQ);

			// adjustment factor (prevent removing more water than available)
			__m512 waterheight = _mm512_loadu_ps(&g.water_height[idx]);
			__m512 waterVolume = _mm512_mul_ps(waterheight, _mm512_set1_ps(pLL * pLL));
			__m512 outVolume = _mm512_mul_ps(sum, _mm512_set1_ps(s->K_dt));
			__m512 adjustmentFactor = _mm512_min_ps(_mm512_set1_ps(1.f), _mm512_div_ps(waterVolume, outVolume));
			adjustmentFactor = _mm512_mask_mov_ps(zero, mask, adjustmentFactor);

			// multiply flux by adjustment factor
			curFluxL = _mm512_mul_ps(curFluxL, adjustmentFactor);
			curFluxR = _mm512_mul_ps(curFluxR, adjustmentFactor);
			curFluxU = _mm512_mul_ps(curFluxU, adjustmentFactor);
			curFluxD = _mm512_mul_ps(curFluxD, adjustmentFactor);

			// store back
			_mm512_storeu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 0, curFluxL);
			_mm512_storeu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 1, curFluxR);
			_mm512_storeu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 2, curFluxU);
			_mm512_storeu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 3, curFluxD);
		}
	}

	ero2_simd_fix_borders(g);
}

void Euler::ero3_simd_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;


	const __m512 KDT = _mm512_set1_ps(s->K_dt);
	const __m512 PLL = _mm512_set1_ps(pLL);
	const __m512 PLL_x_PLL = _mm512_set1_ps(pLL * pLL);
	const __m512 ZERO = _mm512_setzero_ps();
	const __m512 HALF = _mm512_set1_ps(0.5f);

	if (!s->e_flow)
		return;

	// 3. water height
	PARALLELIZE_LOOP_SIMD
	for (int y = 1; y < h - 1; y++)
	{
		int row = y * w;
		for (int x = 1; x < w - 16; x += 16)
		{
			auto idx = row + x;

			// flux from neighbors
			__m512 inFluxL = _mm512_loadu_ps(((float*)g.flux.data()) + idx - 1 + g.flux.size() * 1);
			__m512 inFluxR = _mm512_loadu_ps(((float*)g.flux.data()) + idx + 1 + g.flux.size() * 0);
			__m512 inFluxU = _mm512_loadu_ps(((float*)g.flux.data()) + idx - w + g.flux.size() * 3);
			__m512 inFluxD = _mm512_loadu_ps(((float*)g.flux.data()) + idx + w + g.flux.size() * 2);
			__m512 sumIn = _mm512_add_ps(inFluxL, _mm512_add_ps(inFluxR, _mm512_add_ps(inFluxU, inFluxD)));

			// flux out
			__m512 outFluxL = _mm512_loadu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 0);
			__m512 outFluxR = _mm512_loadu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 1);
			__m512 outFluxU = _mm512_loadu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 2);
			__m512 outFluxD = _mm512_loadu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 3);
			__m512 sumOut = _mm512_add_ps(outFluxL, _mm512_add_ps(outFluxR, _mm512_add_ps(outFluxU, outFluxD)));

			// deltaV = (sumIn - sumOut) * dt
			__m512 deltaV = _mm512_mul_ps(_mm512_sub_ps(sumIn, sumOut), KDT);
			// deltaH = deltaV / (pLL * pLL)
			__m512 deltaH = _mm512_div_ps(deltaV, PLL_x_PLL);
			// new water height = max(0, old + deltaH)
			__m512 curWater = _mm512_loadu_ps(&g.water_height[idx]);
			__m512 newWater = _mm512_max_ps(ZERO, _mm512_add_ps(curWater, deltaH));
			_mm512_storeu_ps(&g.water_height[idx], newWater);
			// meanH = (newWater + curWater) / 2
			__m512 meanH = _mm512_mul_ps(_mm512_add_ps(newWater, curWater), HALF);


			// fluxX = leftIn - curFluxL + curFluxR - rightIn
			__m512 fluxX = _mm512_sub_ps(_mm512_add_ps(inFluxL, _mm512_sub_ps(outFluxR, inFluxR)), outFluxL);
			// fluxY = upIn - curFluxU + curFluxD - downIn
			__m512 fluxY = _mm512_sub_ps(_mm512_add_ps(inFluxU, _mm512_sub_ps(outFluxD, inFluxD)), outFluxU);

			// velocity = flux / (meanH * pLL)
			__m512 meanH_x_PLL = _mm512_mul_ps(meanH, PLL);
			fluxX = _mm512_div_ps(fluxX, meanH_x_PLL);
			fluxY = _mm512_div_ps(fluxY, meanH_x_PLL);

			// set flux to 0 where meanH <= 0
			__mmask16 mask = _mm512_cmp_ps_mask(meanH, ZERO, _CMP_GT_OQ);
			fluxX = _mm512_mask_mov_ps(ZERO, mask, fluxX);
			fluxY = _mm512_mask_mov_ps(ZERO, mask, fluxY);

			// store velocity
			_mm512_storeu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 0, fluxX);
			_mm512_storeu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 1, fluxY);
		}
		
	}
	ero3_simd_fix_borders(g);
}

void Euler::ero4_simd_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	const int w = g.width;
	const int h = g.height;
	constexpr gfloat erosionClamp = 10.0f;


	static AVector<gfloat> new_terrain_height;
	if (new_terrain_height.size() != g.terrain_height.size())
	{
		new_terrain_height.resize(g.terrain_height.size());
		memcpy(new_terrain_height.data(), g.terrain_height.data(), sizeof(gfloat) * g.terrain_height.size());
	}

	if (!s->e_erosion)
		return;
	PARALLELIZE_LOOP_SIMD
	for (int y = 1; y < h - 1; ++y)
	{
		const int yw = y * w;
		for (int x = 1; x < w - 16; x += 16)
		{
			const int idx = yw + x;

			// gradX and gradY
			__m512 terrainL = _mm512_loadu_ps(&g.terrain_height[idx - 1]);
			__m512 terrainR = _mm512_loadu_ps(&g.terrain_height[idx + 1]);
			__m512 terrainU = _mm512_loadu_ps(&g.terrain_height[idx - w]);
			__m512 terrainD = _mm512_loadu_ps(&g.terrain_height[idx + w]);

			__m512 gradX = _mm512_mul_ps(_mm512_sub_ps(terrainR, terrainL), _mm512_set1_ps(0.5f));
			__m512 gradY = _mm512_mul_ps(_mm512_sub_ps(terrainD, terrainU), _mm512_set1_ps(0.5f));

			// Compute vx*vx + vy*vy
			__m512 sumSq = _mm512_fmadd_ps(gradX, gradX, _mm512_mul_ps(gradY, gradY));

			// local tilt = sumSq / (1 + sumSq)
			__m512 tilt = _mm512_div_ps(sumSq, _mm512_add_ps(_mm512_set1_ps(1.0f), sumSq));

			// clamp tilt to minimum
			tilt = _mm512_max_ps(tilt, _mm512_set1_ps(s->K_tilt_minimum));

			// sqrt
			//__m512 velocityLen = _mm512_sqrt_ps(tilt);


			//gfloat grade = glm::clamp(gradX * gradX + gradY * gradY, -erosionClamp, erosionClamp);
			//gfloat sin_local_tilt = glm::sqrt(grade / (1.0f + grade));
			//sin_local_tilt = glm::max(sin_local_tilt, s->K_tilt_minimum);
			//gfloat velLen = glm::length(g.velocity[idx]);

			__m512 velX = _mm512_loadu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 0);
			__m512 velY = _mm512_loadu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 1);
			__m512 velocityLen = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(velX, velX), _mm512_mul_ps(velY, velY)));

			// capacity = s->K_sediment_capacity * velLen * sin_local_tilt * glm::min(1.0f, g.water_height[idx]);
			__m512 capacity = _mm512_mul_ps(velocityLen,
			                                _mm512_mul_ps(_mm512_set1_ps(s->K_sediment_capacity),
			                                              _mm512_mul_ps(tilt,
			                                                            _mm512_min_ps(
				                                                            _mm512_set1_ps(1.0f),
				                                                            _mm512_loadu_ps(&g.water_height[idx])))));

			// perlin
			__m512 perlinFactor = _mm512_loadu_ps(&perlinMap[idx]);
			// depth factor
			__m512 depthDiff = _mm512_sub_ps(_mm512_loadu_ps(&originalHeight[idx]),
			                                 _mm512_loadu_ps(&g.terrain_height[idx]));

			// (depthDiff < 0.0f) ? 1.0f : 1.0f / (1.0f + depthDiff)
			__mmask16 mask = _mm512_cmp_ps_mask(depthDiff, _mm512_set1_ps(0.0f), _CMP_GT_OQ);
			__m512 depthFactor = _mm512_div_ps(_mm512_set1_ps(1.0f), _mm512_add_ps(_mm512_set1_ps(1.0f), depthDiff));
			depthFactor = _mm512_mask_mov_ps(_mm512_set1_ps(1.0f), mask, depthFactor);

			__m512 sediment = _mm512_loadu_ps(&g.sediment[idx]);


			// ====== now dissolve or deposit in case capacity > sediment or not

			// Compare mask: capacity > sediment → dissolving
			__mmask16 maskDissolve = _mm512_cmp_ps_mask(capacity, sediment, _CMP_GT_OQ);

			// Shared absolute difference
			__m512 diff = _mm512_abs_ps(_mm512_sub_ps(capacity, sediment)); // shared magnitude for both branches

			// Base dSoil = K * diff
			__m512 k_mix = _mm512_mask_blend_ps(maskDissolve, _mm512_set1_ps(s->K_d_depositing),
			                                    _mm512_set1_ps(s->K_s_dissolving));
			__m512 dSoil = _mm512_mul_ps(k_mix, diff);

			// If dissolving, multiply by perlin and depth factor
			dSoil = _mm512_mask_mul_ps(dSoil, maskDissolve, dSoil, _mm512_mul_ps(perlinFactor, depthFactor));

			// Clamp to max dissolve
			dSoil = _mm512_min_ps(dSoil, _mm512_set1_ps(pMaxDissolve));
			// If dissolving, clamp to terrain height
			__m512 terrainHeight = _mm512_loadu_ps(&g.terrain_height[idx]);
			dSoil = _mm512_mask_min_ps(dSoil, maskDissolve, dSoil, terrainHeight);

			// make dSoil negative if depositing
			dSoil = _mm512_mask_mov_ps(dSoil, maskDissolve, _mm512_sub_ps(_mm512_set1_ps(0.0f), dSoil));
			// update terrain height and sediment
			//_mm512_storeu_ps(&g.terrain_height[idx], _mm512_add_ps(terrainHeight, dSoil));
			_mm512_storeu_ps(&new_terrain_height[idx], _mm512_add_ps(terrainHeight, dSoil));
			_mm512_storeu_ps(&g.sediment[idx], _mm512_sub_ps(sediment, dSoil));


			//if (capacity > g.sediment[idx])
			//{
			//	gfloat dSoil = s->K_s_dissolving * (capacity - g.sediment[idx]) * perlinFactor * depthFactor;
			//	dSoil = glm::min(dSoil, pMaxDissolve);
			//	dSoil = glm::min(dSoil, g.terrain_height[idx]);
			//
			//	g.terrain_height[idx] -= dSoil;
			//	g.sediment[idx] += dSoil;
			//}
			//else
			//{
			//	gfloat dSoil = s->K_d_depositing * (g.sediment[idx] - capacity);
			//	dSoil = glm::min(dSoil, pMaxDissolve);
			//
			//	g.terrain_height[idx] += dSoil;
			//	g.sediment[idx] -= dSoil;
			//}
		}
	}

	// lets swap terrain height buffers
	std::swap(new_terrain_height, g.terrain_height);
}

void Euler::ero5_simd_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	const int w = g.width;
	const int h = g.height;

	const __m512 kdt = _mm512_set1_ps(s->K_dt);


	static AVector<gfloat> new_sediment;
	if (new_sediment.size() != g.sediment.size())
	{
		new_sediment.resize(g.sediment.size());
		ZeroMemory(new_sediment.data(), sizeof(gfloat) * g.sediment.size());
	}

	std::array xIncrements = {
		0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f,
		8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f
	};
	__m512 xInc = _mm512_loadu_ps(xIncrements.data());

	if (!s->e_erosion)
		return;

	PARALLELIZE_LOOP_SIMD
	for (int y = 1; y < h - 1; ++y)
	{
		const int yw = y * w;
		for (int x = 1; x < w - 16; x += 16)
		{
			const int idx = yw + x;

			__m512 velX = _mm512_loadu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 0);
			__m512 velY = _mm512_loadu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 1);

			velX = _mm512_mul_ps(velX, kdt);
			velY = _mm512_mul_ps(velY, kdt);

			// target position = current position - velocity * dt

			__m512 xIndices = _mm512_add_ps(_mm512_set1_ps((gfloat)x), xInc);
			__m512 fx = _mm512_sub_ps(xIndices, velX);

			__m512 fy = _mm512_sub_ps(_mm512_set1_ps((gfloat)y), velY);

			__m512 sediment = ter::interpolate2D(g.sediment.data(), w, h, fx, fy);

			_mm512_storeu_ps(&new_sediment[idx], sediment);
		}
	}
	// swap sediment buffers
	std::swap(new_sediment, g.sediment);
}

void Euler::ero6_simd_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	const int w = g.width;
	const int h = g.height;

	const gfloat evapFactor = 1.0f - s->K_evaporation * s->K_dt;
	constexpr gfloat evaporationEpsilon = 0.001f;

	const __m512 EVAP_FACT = _mm512_set1_ps(evapFactor);
	const __m512 EPSILON = _mm512_set1_ps(evaporationEpsilon);
	const __m512 ZERO = _mm512_setzero_ps();

	if (!s->e_evaporation)
		return;

	PARALLELIZE_LOOP_SIMD
	for (int y = 1; y < h - 1; ++y)
	{
		const int yw = y * w;
		for (int x = 0; x < w - 16; x += 16)
		{
			const int idx = yw + x;

			// waterHeight *= evapFactor
			__m512 waterHeight = _mm512_loadu_ps(&g.water_height[idx]);
			waterHeight = _mm512_mul_ps(waterHeight, EVAP_FACT);
			// clamp to 0 if below epsilon
			__mmask16 mask = _mm512_cmp_ps_mask(waterHeight, EPSILON, _CMP_LT_OQ);
			waterHeight = _mm512_mask_mov_ps(waterHeight, mask, ZERO);

			// store
			_mm512_storeu_ps(&g.water_height[idx], waterHeight);
		}
	}
}

void Euler::ero7_simd_old(EulerGround& g)
{
	ND_PROFILE_METHOD();

	const int w = g.width;
	const int h = g.height;

	const gfloat multiplier = s->K_landSlideSpeed * s->K_dt;
	const __m512 K_MULT = _mm512_set1_ps(multiplier);
	const __m512 ZERO = _mm512_setzero_ps();
	const __m512 LAND_SLIDE_CUTOFF_ANGLE = _mm512_set1_ps(s->K_landSlideCutoffAngle);
	__mmask16 interleave = 0b0101010101010101;

	static bool interLeaveFlag = false;
	interLeaveFlag = !interLeaveFlag;
	if (interLeaveFlag)
		interleave = ~interleave;

	if (!s->e_landslide)
		return;

	PARALLELIZE_LOOP_SIMD
	for (int y = 1; y < h - 1; ++y)
	{
		const int yw = y * w;
		for (int x = 1; x < w - 16; x += 16)
		{
			const int idx = yw + x;

			__m512 height = _mm512_loadu_ps(&g.terrain_height[idx]);
			__m512 heightE = _mm512_loadu_ps(&g.terrain_height[idx + 1]); // east neighbor
			__m512 heightS = _mm512_loadu_ps(&g.terrain_height[idx + w]); // south neighbor
			__m512 deltaE = _mm512_sub_ps(heightE, height);
			__m512 deltaS = _mm512_sub_ps(heightS, height);

			__m512 takeNE = _mm512_mul_ps(deltaE, K_MULT);
			__m512 takeNS = _mm512_mul_ps(deltaS, K_MULT);

			__mmask16 signsE = _mm512_cmp_ps_mask(takeNE, ZERO, _CMP_LT_OQ);
			__mmask16 signsS = _mm512_cmp_ps_mask(takeNS, ZERO, _CMP_LT_OQ);

			takeNE = _mm512_max_ps(ZERO, _mm512_sub_ps(_mm512_abs_ps(takeNE), LAND_SLIDE_CUTOFF_ANGLE));
			takeNS = _mm512_max_ps(ZERO, _mm512_sub_ps(_mm512_abs_ps(takeNS), LAND_SLIDE_CUTOFF_ANGLE));


			takeNE = _mm512_mask_sub_ps(takeNE, signsE, ZERO, takeNE);
			takeNS = _mm512_mask_sub_ps(takeNS, signsS, ZERO, takeNS);


			__m512 sum = _mm512_add_ps(height, _mm512_add_ps(takeNE, takeNS));

			_mm512_mask_storeu_ps(&g.terrain_height[idx], interleave, sum);

			// subtract from neighbors
			__m512 heightE_new = _mm512_sub_ps(heightE, takeNE);
			__m512 heightS_new = _mm512_sub_ps(heightS, takeNS);

			_mm512_mask_storeu_ps(&g.terrain_height[idx + 1], interleave, heightE_new);
			_mm512_mask_storeu_ps(&g.terrain_height[idx + w], interleave, heightS_new);
		}
	}
}

void Euler::ero3_simd_fix_borders(EulerGround& g)
{
	auto w = g.width;
	auto h = g.height;

	// now we nullify velocity on borders if needed
	for (int y = 1; y < h - 1; y++)
	{
		int row = y * w;
		auto& velLLeft = *(((float*)g.velocity.data()) + (row + 1) + g.velocity.size() * 0);
		auto& velLRight = *(((float*)g.velocity.data()) + (row + w - 2) + g.velocity.size() * 0);

		velLLeft = std::max(0.f, velLLeft);
		velLRight = std::min(0.f, velLRight);
	}
	// top and bottom
	for (int x = 0; x < w; x++)
	{
		auto& velTop = *(((float*)g.velocity.data()) + (x + w) + g.velocity.size() * 1);
		auto& velBottom = *(((float*)g.velocity.data()) + ((h - 2) * w + x) + g.velocity.size() * 1);
		velTop = std::max(0.f, velTop);
		velBottom = std::min(0.f, velBottom);
	}
}

void Euler::ero2_simd_fix_borders(EulerGround& g)
{
	auto w = g.width;
	auto h = g.height;

	// nullify borders
	// now we need to set outfluxes to zero on borders
	for (int y = 1; y < h - 1; y++)
	{
		int row = y * w;
		*((float*)g.flux.data() + (row + 1) + g.flux.size() * 0) = 0.f;
		*((float*)g.flux.data() + (row + w - 1 - 1) + g.flux.size() * 1) = 0.f;
	}
	// top and bottom
	ZeroMemory(((float*)g.flux.data()) + g.flux.size() * 2 + w, w * sizeof(float));
	ZeroMemory(((float*)g.flux.data()) + g.flux.size() * 3 + (h - 2) * w, w * sizeof(float));
}

void Euler::ero3_fix_borders(EulerGround& g)
{
	auto w = g.width;
	auto h = g.height;

	// now we nullify velocity on borders if needed
	for (int y = 1; y < h - 1; y++)
	{
		int row = y * w;
		auto& velLeft = g.velocity[row + 1].x;
		auto& velRight = g.velocity[row + w - 2].x;
		velLeft = glm::max((gfloat)0, velLeft);
		velRight = glm::min((gfloat)0, velRight);
	}
	// top and bottom
	for (int x = 0; x < w; x++)
	{
		auto& velTop = g.velocity[w + x].y;
		auto& velBottom = g.velocity[(h - 2) * w + x].y;
		velTop = glm::max((gfloat)0, velTop);
		velBottom = glm::min((gfloat)0, velBottom);
	}

}

void Euler::ero2_fix_borders(EulerGround& g)
{
	auto w = g.width;
	auto h = g.height;


	// nullify borders
	// now we need to set outfluxes to zero on borders
	for (int y = 1; y < h - 1; y++)
	{
		int row = y * w;
		g.flux[row+1].x =  0.f;
		g.flux[row + w - 1 - 1].y = 0.f;
		
	}
	// top and bottom
	for (int x = 0; x < w; x++)
	{
		g.flux[w + x].z = 0.f;
		g.flux[(h - 2) * w + x].w = 0.f;
	}
}
