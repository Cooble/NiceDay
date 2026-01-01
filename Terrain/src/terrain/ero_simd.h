// ==================================================================================
// Include Guard Logic for 3-Pass Compilation
// ==================================================================================

// 1. Detect Mode
#if defined(EROSIMD_OMP_ENABLE)
#define CURRENT_MODE_OMP
#ifdef EROSIMD_H_OMP_INCLUDED
#define SKIP_EROSIMD_PASS 1
#endif
#define EROSIMD_H_OMP_INCLUDED

#elif defined(EROSIMD_PARALLEL_ENABLE)
#define CURRENT_MODE_PARALLEL
#ifdef EROSIMD_H_PARALLEL_INCLUDED
#define SKIP_EROSIMD_PASS 1
#endif
#define EROSIMD_H_PARALLEL_INCLUDED

#else
#define CURRENT_MODE_PRIMITIVE
#ifdef EROSIMD_H_PRIMITIVE_INCLUDED
#define SKIP_EROSIMD_PASS 1
#endif
#define EROSIMD_H_PRIMITIVE_INCLUDED
#endif

// ==================================================================================
// Main Body
// ==================================================================================
#ifndef SKIP_EROSIMD_PASS

// --- Common Includes (Protected against re-inclusion by internal guards typically) ---
#include <immintrin.h>
#include <vector>
#include <algorithm>
#include <utility>
#include <array>
#include <cmath>

// Ensure "ero.h" and "EulerSim.h" have their own standard #pragma once or ifndef guards
#include "ero.h"
#include "EulerSim.h"

// --- Mode Specific Defines and Namespaces ---

#if defined(CURRENT_MODE_OMP)
    // *** OpenMP Version ***
#include <omp.h>

namespace EroOmp {
    // _Pragma is the standard C++ way to use #pragma inside a macro
#define PARALLELIZE_LOOP \
            _Pragma("omp parallel for schedule(static)") \
            for (int y = 1; y < h - 1; ++y) {

#define END_PARALLELIZE_LOOP }

#elif defined(CURRENT_MODE_PARALLEL)
    // *** C++17 Parallel STL Version ***
#include <execution>
#include <thread>

namespace EroParallel {
#define PARALLELIZE_LOOP \
            std::for_each(std::execution::par_unseq, g.pc.chunks.begin(), g.pc.chunks.end(), \
                [&](const auto& chunk) { \
                    for (int y = chunk.first; y < chunk.second; ++y) {

#define END_PARALLELIZE_LOOP }});

#else
    // *** Vanilla (Primitive) Version ***
    // No namespace (Global scope) or define one if you prefer

#define PARALLELIZE_LOOP for (int y = 1; y < h - 1; ++y) {
#define END_PARALLELIZE_LOOP }
#endif

// ==================================================================================
// Algorithm Implementation
// ==================================================================================

    // 1. Rain & Evaporation - SIMD (Merged)
inline void ero1_simd(EulerGround& g, Euler::EulerSettings* s)
{
    auto w = g.width;
    auto h = g.height;

    // Rain Constant
    const float rainVal = s->K_dt * s->K_rain;
    const __m512 V_RAIN = _mm512_set1_ps(rainVal);

    // Evaporation Constants
    const float evapFactor = 1.0f - s->K_evaporation * s->K_dt;
    const __m512 V_EVAP = _mm512_set1_ps(evapFactor);

    const __m512 V_EPSILON = _mm512_set1_ps(0.001f);
    const __m512 V_ZERO = _mm512_setzero_ps();

    PARALLELIZE_LOOP
        int offset = y * w;

    for (int x = 1; x < w - 16; x += 16)
    {
        // Load
        __m512 vdata = _mm512_loadu_ps(&g.water_height[offset + x]);

        // 1. Apply Rain (Add)
        vdata = _mm512_add_ps(vdata, V_RAIN);

        // 2. Apply Evaporation (Multiply)
        vdata = _mm512_mul_ps(vdata, V_EVAP);

        // 3. Clean up extremely small values (Epsilon check)
        __mmask16 mask = _mm512_cmp_ps_mask(vdata, V_EPSILON, _CMP_LT_OQ);
        vdata = _mm512_mask_mov_ps(vdata, mask, V_ZERO);

        // Store
        _mm512_storeu_ps(&g.water_height[offset + x], vdata);
    }
    END_PARALLELIZE_LOOP
}

// 2. Flux - SIMD
inline void ero2_simd(EulerGround& g, Euler::EulerSettings* s)
{
    if (!s->e_flow)
        return;

    auto w = g.width;
    auto h = g.height;

    // totalHeight = terrain + water
    PARALLELIZE_LOOP
        int offset = y * w;
    int x;
    for (x = 0; x < w - 16; x += 16)
    {
        __m512 vdataA = _mm512_loadu_ps(&g.terrain_height[offset + x]);
        __m512 vdataB = _mm512_loadu_ps(&g.water_height[offset + x]);
        vdataA = _mm512_add_ps(vdataA, vdataB);
        _mm512_storeu_ps(&g.total_height[offset + x], vdataA);
    }
    // last two elements safety check (assuming w not multiple of 16 handling elsewhere or acceptable)
    // Manual cleanup for edges if needed
    if (offset + x < g.total_height.size()) {
        g.total_height[offset + x] = g.terrain_height[offset + x] + g.water_height[offset + x];
    }
    END_PARALLELIZE_LOOP

        const float fluxFactor = s->K_dt * pPipeArea / pPipeLen * pGravity;
    const __m512 fluxFactorSIMD = _mm512_set1_ps(fluxFactor);

    PARALLELIZE_LOOP
        int rowC = y * w;

    for (int x = 1; x < w - 16; x += 16)
    {
        int idx = rowC + x;
        int idxL = idx - 1;
        int idxR = idx + 1;
        int idxU = idx - w;
        int idxD = idx + w;

        // height load
        __m512 cur = _mm512_loadu_ps(&g.total_height[idx]);
        __m512 left = _mm512_loadu_ps(&g.total_height[idxL]);
        __m512 right = _mm512_loadu_ps(&g.total_height[idxR]);
        __m512 up = _mm512_loadu_ps(&g.total_height[idxU]);
        __m512 down = _mm512_loadu_ps(&g.total_height[idxD]);

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
    END_PARALLELIZE_LOOP
}

// 3. Water height and velocity - SIMD
inline void ero3_simd(EulerGround& g, Euler::EulerSettings* s)
{
    if (!s->e_flow)
        return;

    auto w = g.width;
    auto h = g.height;

    const __m512 KDT = _mm512_set1_ps(s->K_dt);
    const __m512 PLL = _mm512_set1_ps(pLL);
    const __m512 PLL_x_PLL = _mm512_set1_ps(pLL * pLL);
    const __m512 ZERO = _mm512_setzero_ps();
    const __m512 HALF = _mm512_set1_ps(0.5f);

    PARALLELIZE_LOOP
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
    END_PARALLELIZE_LOOP
}

// 4. Erosion and deposition - SIMD
inline void ero4_simd(EulerGround& g, Euler::EulerSettings* s)
{
    if (!s->e_erosion)
        return;

    const int w = g.width;
    const int h = g.height;

    PARALLELIZE_LOOP
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

        // Compute gradX*gradX + gradY*gradY
        __m512 sumSq = _mm512_fmadd_ps(gradX, gradX, _mm512_mul_ps(gradY, gradY));

        // local tilt = sumSq / (1 + sumSq)
        __m512 tilt = _mm512_div_ps(sumSq, _mm512_add_ps(_mm512_set1_ps(1.0f), sumSq));

        // clamp tilt to minimum
        tilt = _mm512_max_ps(tilt, _mm512_set1_ps(s->K_tilt_minimum));

        // velocity length
        __m512 velX = _mm512_loadu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 0);
        __m512 velY = _mm512_loadu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 1);
        __m512 velocityLen = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(velX, velX), _mm512_mul_ps(velY, velY)));

        // capacity = K_sediment_capacity * velLen * tilt * min(1.0, water_height)
        __m512 capacity = _mm512_mul_ps(velocityLen,
            _mm512_mul_ps(_mm512_set1_ps(s->K_sediment_capacity),
                _mm512_mul_ps(tilt,
                    _mm512_min_ps(
                        _mm512_set1_ps(1.0f),
                        _mm512_loadu_ps(&g.water_height[idx])))));

        // perlin
        __m512 perlinFactor = _mm512_loadu_ps(&g.perlin_map[idx]);

        // depth factor
        __m512 depthDiff = _mm512_sub_ps(_mm512_loadu_ps(&g.original_height[idx]),
            _mm512_loadu_ps(&g.terrain_height[idx]));

        // (depthDiff < 0.0f) ? 1.0f : 1.0f / (1.0f + depthDiff)
        __mmask16 mask = _mm512_cmp_ps_mask(depthDiff, _mm512_set1_ps(0.0f), _CMP_GT_OQ);
        __m512 depthFactor = _mm512_div_ps(_mm512_set1_ps(1.0f), _mm512_add_ps(_mm512_set1_ps(1.0f), depthDiff));
        depthFactor = _mm512_mask_mov_ps(_mm512_set1_ps(1.0f), mask, depthFactor);

        __m512 sediment = _mm512_loadu_ps(&g.sediment[idx]);

        // Compare mask: capacity > sediment → dissolving
        __mmask16 maskDissolve = _mm512_cmp_ps_mask(capacity, sediment, _CMP_GT_OQ);

        // Shared absolute difference
        __m512 diff = _mm512_abs_ps(_mm512_sub_ps(capacity, sediment));

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
        _mm512_storeu_ps(&g.new_terrain_height[idx], _mm512_add_ps(terrainHeight, dSoil));
        _mm512_storeu_ps(&g.sediment[idx], _mm512_sub_ps(sediment, dSoil));
    }
    END_PARALLELIZE_LOOP

        // swap terrain height buffers
        std::swap(g.new_terrain_height, g.terrain_height);
}

// 5. Sediment transport - SIMD
inline void ero5_simd(EulerGround& g, Euler::EulerSettings* s)
{
    if (!s->e_erosion)
        return;

    const int w = g.width;
    const int h = g.height;

    const __m512 kdt = _mm512_set1_ps(s->K_dt);

    std::array<float, 16> xIncrements = {
        0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f,
        8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f
    };
    __m512 xInc = _mm512_loadu_ps(xIncrements.data());

    PARALLELIZE_LOOP
        const int yw = y * w;
    for (int x = 1; x < w - 16; x += 16)
    {
        const int idx = yw + x;

        __m512 velX = _mm512_loadu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 0);
        __m512 velY = _mm512_loadu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 1);

        velX = _mm512_mul_ps(velX, kdt);
        velY = _mm512_mul_ps(velY, kdt);

        // target position = current position - velocity * dt
        __m512 xIndices = _mm512_add_ps(_mm512_set1_ps((float)x), xInc);
        __m512 fx = _mm512_sub_ps(xIndices, velX);
        __m512 fy = _mm512_sub_ps(_mm512_set1_ps((float)y), velY);

        __m512 sediment = ter::interpolate2D(g.sediment.data(), w, h, fx, fy);

        _mm512_storeu_ps(&g.new_sediment[idx], sediment);
    }
    END_PARALLELIZE_LOOP

        // swap sediment buffers
        std::swap(g.new_sediment, g.sediment);
}

// 7. Landslide - SIMD
inline void ero7_simd(EulerGround& g, Euler::EulerSettings* s)
{
    if (!s->e_landslide)
        return;

    const int w = g.width;
    const int h = g.height;

    const float multiplier = s->K_landSlideSpeed * s->K_dt;
    const __m512 K_MULT = _mm512_set1_ps(multiplier);
    const __m512 ZERO = _mm512_setzero_ps();
    const __m512 LAND_SLIDE_CUTOFF_ANGLE = _mm512_set1_ps(s->K_landSlideCutoffAngle);
    __mmask16 interleave = 0b0101010101010101;

    // Note: static variables in included headers are tricky. 
    // With namespaces (EroParallel/EroOmp), these become distinct statics.
    // For Primitive/Global, it remains one static.
    static bool interLeaveFlag = false;
    interLeaveFlag = !interLeaveFlag;
    if (interLeaveFlag)
        interleave = ~interleave;

    // Note: Ero7 is NOT parallelized in the original code due to data dependency
    // We keep it strictly sequential here as requested (commented out in original)
    // PARALLELIZE_LOOP
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


// ==================================================================================
// FIX BORDER
// ==================================================================================
inline void ero3_simd_fix_borders(EulerGround& g)
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

inline void ero2_simd_fix_borders(EulerGround& g)
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

// ==================================================================================
// Clean Up
// ==================================================================================

#if defined(CURRENT_MODE_OMP) || defined(CURRENT_MODE_PARALLEL)
} // End Namespace EroOmp or EroParallel
#endif

// Undefine macros to allow next inclusion pass
#undef PARALLELIZE_LOOP
#undef END_PARALLELIZE_LOOP
#undef CURRENT_MODE_OMP
#undef CURRENT_MODE_PARALLEL
#undef CURRENT_MODE_PRIMITIVE

#endif // SKIP_EROSIMD_PASS
#undef SKIP_EROSIMD_PASS