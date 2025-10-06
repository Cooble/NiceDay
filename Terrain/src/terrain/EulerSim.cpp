#include "ndpch.h"
#include "core/Scoper.h"
#include "EulerSim.h"
#include <ImGuiFileDialog.h>

#include "TerrainLayer.h"
#include "TUtils.h"
#include "core/NBT.h"

#include <immintrin.h>

#include <algorithm>

#undef ND_PROFILE_METHOD
#define ND_PROFILE_METHOD()

using namespace nd;

void Euler::init(EulerGround& g)
{
	perlinMap.resize(g.width * g.width);
	sediment = g.sediment;
	originalHeight = g.terrain_height;

	ter::generate2DPerlin(REINTERPRET_AS(std::vector<gfloat>, perlinMap), g.width, g.height);
	ZeroMemory(g.water_height.data(), g.water_height.size() * sizeof(decltype(g.water_height)::value_type));
}


static uint64_t sMeasureFunctionTime = 1;

void measureFunction(std::function<void(EulerGround&)> a1, std::function<void(EulerGround&)> a2, EulerGround& g)
{
	uint64_t micros1, micros2;
	{
		TimerStaper p("");
		for (int i = 0; i < sMeasureFunctionTime; ++i)
			a1(g);
		micros1 = p.getUS();
	}
	{
		TimerStaper p("");
		for (int i = 0; i < sMeasureFunctionTime; ++i)
			a2(g);
		micros2 = p.getUS();
	}
	ND_BUG("SIMD took {} us, BASIC took {} us, SpeedUp of {}", micros1 / sMeasureFunctionTime,
	       micros2 / sMeasureFunctionTime, (gfloat)micros2 / (gfloat)micros1);
}

#define measureFuncFixed(name) measureFunction(std::bind(&Euler::name##_simd, this,std::placeholders::_1), std::bind(&Euler::name,this, std::placeholders::_1), g)

void Euler::step(EulerGround& g)
{
	if (g.height * g.height != sediment.size())
		init(g);

	static bool first = true;
	if (first)
	{
		first = false;
		sMeasureFunctionTime = 1; //warmup
	}
	else
		sMeasureFunctionTime = 1000;


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

void Euler::imguiRender()
{
	using namespace ter;
	ImGui::Checkbox("Rain", &e_rain);
	ImGui::Checkbox("Flow", &e_flow);
	ImGui::Checkbox("Erosion", &e_erosion);
	ImGui::Checkbox("Evaporation", &e_evaporation);
	ImGui::Checkbox("Landslide", &e_landslide);

	ImGui::SeparatorText("Euler Settings");
	ImGui::PushID(this);


	InputGFloat("Gravity", &K_g, 0, 0, "%.6f");
	SliderGFloat("Dt", &K_dt, 0, (gfloat)0.1, "%.6f");
	SliderGFloat("Rain", &K_rain, 0, 10, "%.3f");
	SliderGFloat("Sediment Capacity", &K_sediment_capacity, 0, (gfloat)0.5, "%.6f");
	SliderGFloat("Dissolving", &K_s_dissolving, 0, (gfloat)0.5, "%.6f");
	SliderGFloat("Depositing", &K_d_depositing, 0, (gfloat)0.5, "%.6f");
	SliderGFloat("Evaporation", &K_evaporation, 0, (gfloat)0.5, "%.6f");

	SliderGFloat("Tilt Minimum", &K_tilt_minimum, 0, 10, "%.3f");
	SliderGFloat("Land Slide Speed", &K_landSlideSpeed, 0, 1, "%.3f");
	SliderGFloat("Land Slide Cutoff Angle", &K_landSlideCutoffAngle, 0, 1, "%.3f");

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
}

void Euler::save(nd::NBT& src)
{
	NBT_SAVE(src, K_dt);
	NBT_SAVE(src, K_rain);
	NBT_SAVE(src, K_g);
	NBT_SAVE(src, K_sediment_capacity);
	NBT_SAVE(src, K_s_dissolving);
	NBT_SAVE(src, K_d_depositing);
	NBT_SAVE(src, K_evaporation);
	NBT_SAVE(src, K_tilt_minimum);
	NBT_SAVE(src, K_landSlideSpeed);
	NBT_SAVE(src, K_landSlideCutoffAngle);
}

void Euler::load(nd::NBT& src)
{
	NBT_LOAD(src, K_dt);
	NBT_LOAD(src, K_rain);
	NBT_LOAD(src, K_g);
	NBT_LOAD(src, K_sediment_capacity);
	NBT_LOAD(src, K_s_dissolving);
	NBT_LOAD(src, K_d_depositing);
	NBT_LOAD(src, K_evaporation);
	NBT_LOAD(src, K_tilt_minimum);
	NBT_LOAD(src, K_landSlideSpeed);
	NBT_LOAD(src, K_landSlideCutoffAngle);
}

void Euler::ero1(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;


	// 1. rain
	for (int y = 1; y < h - 1 && e_rain; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto d = glm::ivec2(x, y) - glm::ivec2(w / 2);

			gfloat increase = (d.x * d.x + d.y * d.y < 100 || (x > w / 2 - 10 && x < w / 2 + 10))
				                  ? K_rain
				                  : 0;

			// rain everywhere same
			increase = K_rain;
			g.water_height[x + y * w] += K_dt * increase;
		}
}

void Euler::ero1_simd(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	const int stride = w;
	const float val = K_dt * K_rain;

	__m512 vval = _mm512_set1_ps(val);

	for (int y = 1; y < h - 1; y++)
	{
		int offset = y * stride + 1;
		int x = 1;

		// process 16 floats at a time
		for (; x <= w - 17; x += 16)
		{
			__m512 vdata = _mm512_loadu_ps(&g.water_height[offset + x]);
			vdata = _mm512_add_ps(vdata, vval);
			_mm512_storeu_ps(&g.water_height[offset + x], vdata);
		}

		// remainder (mask tail if you want)
		for (; x < w - 1; x++)
		{
			g.water_height[offset + x] += val;
		}
	}
}

void Euler::ero2(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;


	static std::vector<gfloat> totalHeight(g.terrain_height.size());
	if (totalHeight.size() != g.terrain_height.size())
		totalHeight.resize(g.terrain_height.size());

	for (size_t i = 0; i < totalHeight.size(); i++)
		totalHeight[i] += g.water_height[i];

	// 2. flux
	for (int y = 1; y < h - 1 && e_flow; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;
			auto deltaH = gvec4(
				totalHeight[idx] - totalHeight[y * w + x - 1],
				totalHeight[idx] - totalHeight[y * w + x + 1],
				totalHeight[idx] - totalHeight[(y - 1) * w + x],
				totalHeight[idx] - totalHeight[(y + 1) * w + x]);

			auto fluxFactor = K_dt * pPipeArea / pPipeLen * pGravity;
			g.flux[idx] = glm::max(gvec4(0.f), g.flux[idx] + deltaH * fluxFactor);

			auto sumF = glm::compAdd(g.flux[idx]);

			if (sumF > 0)
			{
				auto waterVolume = g.water_height[idx] * pLL * pLL;
				auto outVolume = sumF * K_dt;
				auto adjustmentFactor = glm::min((gfloat)1, waterVolume / outVolume);

				g.flux[idx] *= adjustmentFactor;
			}
		}
}

void Euler::ero3(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;


	// 3. water height
	for (int y = 1; y < h - 1 && e_flow; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;
			auto sumIn =
				+g.flux[y * w + x - 1].y
				+ g.flux[y * w + x + 1].x
				+ g.flux[(y - 1) * w + x].w
				+ g.flux[(y + 1) * w + x].z;
			auto sumOut = glm::compAdd(g.flux[idx]);

			auto deltaV = (sumIn - sumOut) * K_dt;
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
}

void Euler::ero4(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	// 4. erosion
	constexpr gfloat erosionClamp = 10;

	for (int y = 1; y < h - 1 && e_erosion; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;

			auto gradX = (g.terrain_height[y * w + x + 1] - g.terrain_height[y * w + x - 1]) / 2;
			auto gradY = (g.terrain_height[(y + 1) * w + x] - g.terrain_height[(y - 1) * w + x]) / 2;

			auto grade = glm::clamp(gradX * gradX + gradY * gradY, -erosionClamp, erosionClamp);
			auto sin_local_tilt = glm::sqrt(grade / (1 + grade));

			sin_local_tilt = glm::max(sin_local_tilt, K_tilt_minimum);

			auto capacity = K_sediment_capacity * glm::length(g.velocity[idx]) * sin_local_tilt * glm::min(
				(gfloat)1, g.water_height[idx]);


			auto perlinFactor = perlinMap[idx];

			// the deeper the harder to dissolve
			auto depthFactor = 1 / (1 + originalHeight[idx] - g.terrain_height[idx]);
			if (originalHeight[idx] - g.terrain_height[idx] < 0)
				depthFactor = 1;

			if (capacity > g.sediment[idx])
			{
				auto dSoil = K_s_dissolving * (capacity - g.sediment[idx]) * perlinFactor * depthFactor;

				// limit dissolve
				dSoil = glm::min(dSoil, pMaxDissolve);

				// limit dissolve to the terrain height
				dSoil = glm::min(dSoil, g.terrain_height[idx]);

				g.terrain_height[idx] = g.terrain_height[idx] - dSoil;
				g.sediment[idx] = g.sediment[idx] + dSoil;
			}
			else
			{
				auto dSoil = K_d_depositing * (g.sediment[idx] - capacity);

				// limit deposit
				dSoil = glm::min(dSoil, pMaxDissolve);

				g.terrain_height[idx] = g.terrain_height[idx] + dSoil;
				g.sediment[idx] = g.sediment[idx] - dSoil;
			}
		}
}

void Euler::ero5(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	// 5. sediment transport
	for (int y = 1; y < h - 1 && e_erosion; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;

			gfloat velx = g.velocity[idx].x;
			gfloat vely = g.velocity[idx].y;

			gfloat fx = (gfloat)x - velx * K_dt;
			gfloat fy = (gfloat)y - vely * K_dt;


			g.sediment[idx] = ter::interpolate2D(REINTERPRET_AS(std::vector<gfloat>, g.sediment), w, h, fx, fy);
			//g.sediment[idx] = g.sediment[idx];
		}
}

void Euler::ero6(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;


	// 6. evaporation
	for (int y = 1; y < h - 1 && e_evaporation; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;
			g.water_height[idx] *= 1 - K_evaporation * K_dt;

			// remove the incredibly small values
			constexpr gfloat evaporationEpsilon = 0.001;
			if (g.water_height[idx] < evaporationEpsilon)
				g.water_height[idx] = 0;
		}
}

void Euler::ero7(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	// 7. landslide
	for (int y = 1; y < h - 1 && e_landslide; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;

			auto& height = g.terrain_height[idx];

			auto heightN = gvec2(
				g.terrain_height[idx + 1],
				g.terrain_height[idx + w]);

			auto delta = heightN - height;

			auto takeN = K_landSlideSpeed * K_dt * delta;

			auto signs = glm::sign(takeN);
			takeN = glm::max(glm::abs(takeN) - K_landSlideCutoffAngle, (gfloat)0.f) * signs;
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


void Euler::ero2_simd(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	static std::vector<gfloat> totalHeight(g.terrain_height.size());
	if (totalHeight.size() != g.terrain_height.size())
		totalHeight.resize(g.terrain_height.size());

	// totalHeight = terrain + water
	for (int y = 0; y < h; y++)
	{
		int offset = y * w;

		// process 16 floats at a time
		for (int x = 0; x < w; x += 16)
		{
			__m512 vdataA = _mm512_load_ps(&g.terrain_height[offset + x]);
			__m512 vdataB = _mm512_load_ps(&g.water_height[offset + x]);
			vdataA = _mm512_add_ps(vdataA, vdataB);
			_mm512_store_ps(&totalHeight[offset + x], vdataA);
		}
	}
	if (!e_flow) return;

	const float fluxFactor = K_dt * pPipeArea / pPipeLen * pGravity;
	const __m512 fluxFactorSIMD = _mm512_set1_ps(fluxFactor);


	for (int y = 1; y < h - 1; ++y)
	{
		int rowU = (y - 1) * w;
		int rowC = y * w;
		int rowD = (y + 1) * w;

		//for (int x = 1; x < w - 1; ++x)
		for (int x = 1; x < w - 1 - 16; x += 16)
		{
			int idx = rowC + x;
			int idxL = idx - 1;
			int idxR = idx + 1;
			int idxU = rowU + x;
			int idxD = rowD + x;


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
			__m512 zero = _mm512_set1_ps(0.f);
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
			__m512 outVolume = _mm512_mul_ps(sum, _mm512_set1_ps(K_dt));
			__m512 adjustmentFactor = _mm512_min_ps(_mm512_set1_ps(1.f), _mm512_div_ps(waterVolume, outVolume));
			adjustmentFactor = _mm512_mask_mov_ps(_mm512_set1_ps(1.f), ~mask, adjustmentFactor);

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
}


void Euler::ero3_simd(EulerGround& g)
{
	ND_PROFILE_METHOD();

	auto w = g.width;
	auto h = g.height;

	// 3. water height
	for (int y = 1; y < h - 1 && e_flow; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;

			auto leftIn = g.flux[idx - 1].y;
			auto rightIn = g.flux[idx + 1].x;
			auto upIn = g.flux[idx - w].w;
			auto downIn = g.flux[idx + w].z;

			auto sumIn = leftIn + rightIn + upIn + downIn;
			auto sumOut = g.flux[idx].x + g.flux[idx].y + g.flux[idx].z + g.flux[idx].w;


			auto deltaV = (sumIn - sumOut) * K_dt;
			auto deltaH = deltaV / (pLL * pLL);
			g.water_height[idx] = glm::max((gfloat)0.f, g.water_height[idx] + deltaH);
			auto meanH = g.water_height[idx] - deltaH / 2.f;

			if (meanH > 0)
			{
				auto fluxX =
					+leftIn
					- g.flux[idx].x
					+ g.flux[idx].y
					- rightIn;
				auto fluxY =
					+upIn
					- g.flux[idx].z
					+ g.flux[idx].w
					- downIn;
				g.velocity[idx] = gvec2(fluxX, fluxY) / (meanH * pLL);
			}
			else
				g.velocity[idx] = gvec2(0.f);
		}
}

#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <algorithm>

void Euler::ero4_simd(EulerGround& g)
{
	ND_PROFILE_METHOD();

	const int w = g.width;
	const int h = g.height;
	constexpr gfloat erosionClamp = 10.0f;

	for (int y = 1; y < h - 1 && e_erosion; ++y)
	{
		const int yw = y * w;
		const int ym1w = (y - 1) * w;
		const int yp1w = (y + 1) * w;

		for (int x = 1; x < w - 1; ++x)
		{
			const int idx = yw + x;

			gfloat gradX = (g.terrain_height[yw + x + 1] - g.terrain_height[yw + x - 1]) * 0.5f;
			gfloat gradY = (g.terrain_height[yp1w + x] - g.terrain_height[ym1w + x]) * 0.5f;

			gfloat grade = glm::clamp(gradX * gradX + gradY * gradY, -erosionClamp, erosionClamp);

			gfloat sin_local_tilt = glm::sqrt(grade / (1.0f + grade));
			sin_local_tilt = glm::max(sin_local_tilt, K_tilt_minimum);

			gfloat velLen = glm::length(g.velocity[idx]);

			gfloat capacity = K_sediment_capacity * velLen * sin_local_tilt *
				glm::min(1.0f, g.water_height[idx]);

			gfloat perlinFactor = perlinMap[idx];
			gfloat depthDiff = originalHeight[idx] - g.terrain_height[idx];
			gfloat depthFactor = (depthDiff < 0.0f) ? 1.0f : 1.0f / (1.0f + depthDiff);

			if (capacity > g.sediment[idx])
			{
				gfloat dSoil = K_s_dissolving * (capacity - g.sediment[idx]) * perlinFactor * depthFactor;
				dSoil = glm::min(dSoil, pMaxDissolve);
				dSoil = glm::min(dSoil, g.terrain_height[idx]);

				g.terrain_height[idx] -= dSoil;
				g.sediment[idx] += dSoil;
			}
			else
			{
				gfloat dSoil = K_d_depositing * (g.sediment[idx] - capacity);
				dSoil = glm::min(dSoil, pMaxDissolve);

				g.terrain_height[idx] += dSoil;
				g.sediment[idx] -= dSoil;
			}
		}
	}
}


void Euler::ero5_simd(EulerGround& g)
{
	ND_PROFILE_METHOD();

	const int w = g.width;
	const int h = g.height;

	for (int y = 1; y < h - 1 && e_erosion; ++y)
	{
		const int yw = y * w;
		for (int x = 1; x < w - 1; ++x)
		{
			const int idx = yw + x;

			gfloat velx = g.velocity[idx].x;
			gfloat vely = g.velocity[idx].y;

			gfloat fx = (gfloat)x - velx * K_dt;
			gfloat fy = (gfloat)y - vely * K_dt;


			g.sediment[idx] = ter::interpolate2D(REINTERPRET_AS(std::vector<gfloat>, g.sediment), w, h, fx, fy);
		}
	}
}

void Euler::ero6_simd(EulerGround& g)
{
	ND_PROFILE_METHOD();

	const int w = g.width;
	const int h = g.height;

	const gfloat evapFactor = 1.0f - K_evaporation * K_dt;
	constexpr gfloat evaporationEpsilon = 0.001f;

	for (int y = 1; y < h - 1 && e_evaporation; ++y)
	{
		const int yw = y * w;
		for (int x = 1; x < w - 1; ++x)
		{
			const int idx = yw + x;

			g.water_height[idx] *= evapFactor;
			if (g.water_height[idx] < evaporationEpsilon)
				g.water_height[idx] = 0.0f;
		}
	}
}

void Euler::ero7_simd(EulerGround& g)
{
	ND_PROFILE_METHOD();

	const int w = g.width;
	const int h = g.height;

	for (int y = 1; y < h - 1 && e_landslide; ++y)
	{
		const int yw = y * w;
		const int yp1w = (y + 1) * w;

		for (int x = 1; x < w - 1; ++x)
		{
			const int idx = yw + x;

			gfloat& height = g.terrain_height[idx];

			gvec2 heightN(
				g.terrain_height[yw + x + 1], // east neighbor
				g.terrain_height[yp1w + x] // south neighbor
			);

			gvec2 delta = heightN - gvec2(height);
			gvec2 takeN = delta * (K_landSlideSpeed * K_dt);

			gvec2 signs = glm::sign(takeN);
			takeN = glm::max(glm::abs(takeN) - K_landSlideCutoffAngle, 0.0f) * signs;

			height += glm::compAdd(takeN);
			g.terrain_height[yw + x + 1] -= takeN.x;
			g.terrain_height[yp1w + x] -= takeN.y;
		}
	}
}
