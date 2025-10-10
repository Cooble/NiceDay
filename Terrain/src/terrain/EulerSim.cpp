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

#ifdef SIMDD
	ero1_simd(g);
	ero2_simd(g);
	ero3_simd(g);
	ero4_simd(g);
	ero5_simd(g);
	ero6_simd(g);
	ero7_simd(g);
#else
	//ero1(g);
	//ero2(g);
	//ero3(g);
	//ero4(g);
	//ero5(g);
	//ero6(g);
	//ero7(g);

	auto w = g.width;
	auto h = g.height;


	// 1. rain
	for (int y = 1; y < h - 1; y++)
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

	static AVector<gfloat> totalHeight;
	if (totalHeight.size()!= g.terrain_height.size())
		totalHeight.resize(g.terrain_height.size());

	for (size_t i = 0; i < totalHeight.size(); i++)
		totalHeight[i] = g.terrain_height[i]+g.water_height[i];

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

	// 5. sediment transport
	for (int y = 1; y < h - 1 && e_erosion; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;

			gfloat velx = g.velocity[idx].x;
			gfloat vely = g.velocity[idx].y;

			gfloat fx = (gfloat)x - velx * K_dt;
			gfloat fy = (gfloat)y - vely * K_dt;


			g.sediment[idx] = ter::interpolate2D(REINTERPRET_AS(std::vector<gfloat>,g.sediment), w, h, fx, fy);
			//g.sediment[idx] = g.sediment[idx];
		}

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

	// 7. landslide
	for (int y = 1; y < h - 1 && e_landslide; y++)
		for (int x = 1; x < w - 1; x++)
		{
			auto idx = y * w + x;

			auto& height = g.terrain_height[idx];

			auto heightN = gvec2(
				g.terrain_height[y * g.width + x + 1],
				g.terrain_height[(y + 1) * g.width + x]);

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
			g.terrain_height[y * w + x + 1] -= takeN.x;
			g.terrain_height[(y + 1) * w + x] -= takeN.y;
		}
#endif
	return;

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
		totalHeight[i] = g.water_height[i] + g.terrain_height[i];

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

			//auto sumF = glm::compAdd(g.flux[idx]);
			auto sumF = g.flux[idx].x + g.flux[idx].y + g.flux[idx].z + g.flux[idx].w;

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
			auto sumOut = g.flux[idx].x + g.flux[idx].y + g.flux[idx].z + g.flux[idx].w;

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
		for (int x = 1; x < w - 1 - 16; x += 16)
		{
			auto idx = y * w + x;

			// flux from neighbors
			__m512 leftIn = _mm512_loadu_ps(((float*)g.flux.data()) + idx - 1 + g.flux.size() * 1);
			__m512 rightIn = _mm512_loadu_ps(((float*)g.flux.data()) + idx + 1 + g.flux.size() * 0);
			__m512 upIn = _mm512_loadu_ps(((float*)g.flux.data()) + idx - w + g.flux.size() * 3);
			__m512 downIn = _mm512_loadu_ps(((float*)g.flux.data()) + idx + w + g.flux.size() * 2);
			__m512 sumIn = _mm512_add_ps(leftIn, _mm512_add_ps(rightIn, _mm512_add_ps(upIn, downIn)));

			// flux out
			__m512 curFluxL = _mm512_loadu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 0);
			__m512 curFluxR = _mm512_loadu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 1);
			__m512 curFluxU = _mm512_loadu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 2);
			__m512 curFluxD = _mm512_loadu_ps(((float*)g.flux.data()) + idx + g.flux.size() * 3);
			__m512 sumOut = _mm512_add_ps(curFluxL, _mm512_add_ps(curFluxR, _mm512_add_ps(curFluxU, curFluxD)));

			// deltaV = (sumIn - sumOut) * dt
			__m512 deltaV = _mm512_mul_ps(_mm512_sub_ps(sumIn, sumOut), _mm512_set1_ps(K_dt));
			// deltaH = deltaV / (pLL * pLL)
			__m512 deltaH = _mm512_div_ps(deltaV, _mm512_set1_ps(pLL * pLL));
			// new water height = max(0, old + deltaH)
			__m512 curWater = _mm512_loadu_ps(&g.water_height[idx]);
			curWater = _mm512_max_ps(_mm512_set1_ps(0.f), _mm512_add_ps(curWater, deltaH));
			_mm512_storeu_ps(&g.water_height[idx], curWater);
			// meanH = water - deltaH/2
			__m512 meanH = _mm512_sub_ps(curWater, _mm512_mul_ps(deltaH, _mm512_set1_ps(0.5f)));


			// fluxX = leftIn - curFluxL + curFluxR - rightIn
			__m512 fluxX = _mm512_sub_ps(_mm512_add_ps(leftIn, _mm512_sub_ps(curFluxR, rightIn)), curFluxL);
			// fluxY = upIn - curFluxU + curFluxD - downIn
			__m512 fluxY = _mm512_sub_ps(_mm512_add_ps(upIn, _mm512_sub_ps(curFluxD, downIn)), curFluxU);

			// velocity = flux / (meanH * pLL)
			fluxX = _mm512_div_ps(fluxX, _mm512_mul_ps(meanH, _mm512_set1_ps(pLL)));
			fluxY = _mm512_div_ps(fluxY, _mm512_mul_ps(meanH, _mm512_set1_ps(pLL)));

			// set flux to 0 where meanH <= 0
			__mmask16 mask = _mm512_cmp_ps_mask(meanH, _mm512_set1_ps(0.f), _CMP_GT_OQ);
			fluxX = _mm512_mask_mov_ps(_mm512_set1_ps(0.f), ~mask, fluxX);
			fluxY = _mm512_mask_mov_ps(_mm512_set1_ps(0.f), ~mask, fluxY);

			// store velocity
			_mm512_storeu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 0, fluxX);
			_mm512_storeu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 1, fluxY);
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
		const int ym1w = yw - w;
		const int yp1w = yw + w;

		for (int x = 1; x < w - 1 - 16; x += 16)
		{
			const int idx = yw + x;

			// gradX and gradY
			__m512 terrainL = _mm512_loadu_ps(&g.terrain_height[yw + x - 1]);
			__m512 terrainR = _mm512_loadu_ps(&g.terrain_height[yw + x + 1]);
			__m512 terrainU = _mm512_loadu_ps(&g.terrain_height[ym1w + x]);
			__m512 terrainD = _mm512_loadu_ps(&g.terrain_height[yp1w + x]);

			__m512 gradX = _mm512_mul_ps(_mm512_sub_ps(terrainR, terrainL), _mm512_set1_ps(0.5f));
			__m512 gradY = _mm512_mul_ps(_mm512_sub_ps(terrainD, terrainU), _mm512_set1_ps(0.5f));

			// Compute vx*vx + vy*vy
			__m512 sumSq = _mm512_fmadd_ps(gradX, gradX, _mm512_mul_ps(gradY, gradY));

			// local tilt = sumSq / (1 + sumSq)
			__m512 tilt = _mm512_div_ps(sumSq, _mm512_add_ps(_mm512_set1_ps(1.0f), sumSq));

			// clamp tilt to minimum
			tilt = _mm512_max_ps(tilt, _mm512_set1_ps(K_tilt_minimum));

			// sqrt
			__m512 velocityLen = _mm512_sqrt_ps(tilt);


			//gfloat grade = glm::clamp(gradX * gradX + gradY * gradY, -erosionClamp, erosionClamp);
			//gfloat sin_local_tilt = glm::sqrt(grade / (1.0f + grade));
			//sin_local_tilt = glm::max(sin_local_tilt, K_tilt_minimum);
			//gfloat velLen = glm::length(g.velocity[idx]);

			// capacity = K_sediment_capacity * velLen * sin_local_tilt * glm::min(1.0f, g.water_height[idx]);
			__m512 capacity = _mm512_mul_ps(_mm512_set1_ps(K_sediment_capacity),
			                                _mm512_mul_ps(velocityLen,
			                                              _mm512_min_ps(
				                                              _mm512_set1_ps(1.0f),
				                                              _mm512_loadu_ps(&g.water_height[idx]))));

			// perlin
			__m512 perlinFactor = _mm512_loadu_ps(&perlinMap[idx]);
			// depth factor
			__m512 depthDiff = _mm512_sub_ps(_mm512_loadu_ps(&originalHeight[idx]),
			                                 _mm512_loadu_ps(&g.terrain_height[idx]));

			// (depthDiff < 0.0f) ? 1.0f : 1.0f / (1.0f + depthDiff)
			__mmask16 mask = _mm512_cmp_ps_mask(depthDiff, _mm512_set1_ps(0.0f), _CMP_LT_OQ);
			__m512 depthFactor = _mm512_div_ps(_mm512_set1_ps(1.0f), _mm512_add_ps(_mm512_set1_ps(1.0f), depthDiff));
			depthFactor = _mm512_mask_mov_ps(_mm512_set1_ps(1.0f), mask, depthFactor);

			__m512 sediment = _mm512_loadu_ps(&g.sediment[idx]);


			// ====== now dissolve or deposit in case capacity > sediment or not

			// Compare mask: capacity > sediment → dissolving
			__mmask16 maskDissolve = _mm512_cmp_ps_mask(capacity, sediment, _CMP_GT_OQ);

			// Shared absolute difference
			__m512 diff = _mm512_sub_ps(capacity, sediment);
			diff = _mm512_abs_ps(diff); // shared magnitude for both branches

			// Base dSoil = K * diff
			__m512 k_mix = _mm512_mask_blend_ps(maskDissolve, _mm512_set1_ps(K_s_dissolving),
			                                    _mm512_set1_ps(K_d_depositing));
			__m512 dSoil = _mm512_mul_ps(k_mix, diff);

			// If dissolving, multiply by perlin and depth factor
			dSoil = _mm512_mask_mul_ps(dSoil, maskDissolve, dSoil, _mm512_mul_ps(perlinFactor, depthFactor));

			// Clamp to max dissolve
			dSoil = _mm512_min_ps(dSoil, _mm512_set1_ps(pMaxDissolve));
			// If dissolving, clamp to terrain height
			__m512 terrainHeight = _mm512_loadu_ps(&g.terrain_height[idx]);
			dSoil = _mm512_mask_min_ps(dSoil, maskDissolve, dSoil, terrainHeight);

			// make dSoil negative if depositing
			dSoil = _mm512_mask_mov_ps(dSoil, ~maskDissolve, _mm512_sub_ps(_mm512_set1_ps(0.0f), dSoil));
			// update terrain height and sediment
			//_mm512_storeu_ps(&g.terrain_height[idx], _mm512_add_ps(terrainHeight, dSoil));
			//_mm512_storeu_ps(&g.sediment[idx], _mm512_sub_ps(sediment, dSoil));
			_mm512_storeu_ps(&g.terrain_height[idx], _mm512_add_ps(terrainHeight, dSoil));
			_mm512_storeu_ps(&g.sediment[idx], _mm512_sub_ps(sediment, dSoil));


			//if (capacity > g.sediment[idx])
			//{
			//	gfloat dSoil = K_s_dissolving * (capacity - g.sediment[idx]) * perlinFactor * depthFactor;
			//	dSoil = glm::min(dSoil, pMaxDissolve);
			//	dSoil = glm::min(dSoil, g.terrain_height[idx]);
			//
			//	g.terrain_height[idx] -= dSoil;
			//	g.sediment[idx] += dSoil;
			//}
			//else
			//{
			//	gfloat dSoil = K_d_depositing * (g.sediment[idx] - capacity);
			//	dSoil = glm::min(dSoil, pMaxDissolve);
			//
			//	g.terrain_height[idx] += dSoil;
			//	g.sediment[idx] -= dSoil;
			//}
		}
	}
}

/*
#include <immintrin.h>
#include <stddef.h>
#include <math.h>

// Example required constants (replace with your real ones)
extern const float K_tilt_minimum;
extern const float K_sediment_capacity;
extern const float K_s_dissolving;
extern const float K_d_depositing;
extern const float pMaxDissolve;

// Helper: float absolute via bitmask (safe, single instruction)
static inline __m512 mm512_abs_ps_safe(__m512 v) {
	const __m512i absmask = _mm512_set1_epi32(0x7fffffff);
	return _mm512_and_ps(v, _mm512_castsi512_ps(absmask));
}

void Euler::ero4_simd(EulerGround& g)
{
	// ND_PROFILE_METHOD(); -- keep or remove as you like

	const int w = g.width;
	const int h = g.height;

	// constants used in the loop
	const __m512 half = _mm512_set1_ps(0.5f);
	const __m512 one = _mm512_set1_ps(1.0f);
	const __m512 one_f = one;
	const __m512 k_tilt_min = _mm512_set1_ps(K_tilt_minimum);
	const __m512 k_sed_cap = _mm512_set1_ps(K_sediment_capacity);
	const __m512 k_s_diss = _mm512_set1_ps(K_s_dissolving);
	const __m512 k_d_depo = _mm512_set1_ps(K_d_depositing);
	const __m512 maxD_vec = _mm512_set1_ps(pMaxDissolve);
	const __m512 erosionClampMax = _mm512_set1_ps((float)10.0f); // as per your original constexpr

	// component stride for velocity (pattern you described)
	const size_t velCompStride = g.velocity.size(); // number of floats per component offset
	// pointers to raw arrays
	float* terrain_ptr = g.terrain_height.data();
	float* sed_ptr = g.sediment.data();
	float* water_ptr = g.water_height.data();
	float* perlin_ptr = perlinMap.data();
	float* origH_ptr = originalHeight.data();
	float* vel_base = (float*)g.velocity.data(); // base pointer for velocity components

	// iterate rows (avoid borders)
	for (int y = 1; y < h - 1; ++y) {
		// x runs 1 .. w-2
		int x = 1;
		const int x_end = w - 1; // exclusive index for natural loops; we'll stop at w-1
		// vectorized main loop, 16 floats per iteration
		for (; x + 15 <= w - 2; x += 16) {
			const int idx = y * w + x; // base index into 1D arrays for this 16-wide block

			// ---- load terrain neighbors for gradient ----
			// left: idx - 1
			__m512 terrainL = _mm512_loadu_ps(&terrain_ptr[idx - 1]);
			// right: idx + 1
			__m512 terrainR = _mm512_loadu_ps(&terrain_ptr[idx + 1]);
			// up: (y-1)*w + x  -> which is idx - w
			__m512 terrainU = _mm512_loadu_ps(&terrain_ptr[idx - w]);
			// down: (y+1)*w + x -> idx + w
			__m512 terrainD = _mm512_loadu_ps(&terrain_ptr[idx + w]);

			// gradX = (R - L) * 0.5
			__m512 gradX = _mm512_mul_ps(_mm512_sub_ps(terrainR, terrainL), half);
			// gradY = (D - U) * 0.5
			__m512 gradY = _mm512_mul_ps(_mm512_sub_ps(terrainD, terrainU), half);

			// grade = gradX*gradX + gradY*gradY
			__m512 grade = _mm512_fmadd_ps(gradX, gradX, _mm512_mul_ps(gradY, gradY));

			// clamp grade to [-erosionClamp, +erosionClamp]
			// You used glm::clamp(grade, -erosionClamp, erosionClamp)
			__m512 grade_clamped = _mm512_min_ps(grade, erosionClampMax);
			grade_clamped = _mm512_max_ps(grade_clamped, _mm512_sub_ps(_mm512_setzero_ps(), erosionClampMax)); // -10.0f

			// sin_local_tilt = sqrt(grade / (1.0 + grade))
			__m512 denom = _mm512_add_ps(one_f, grade_clamped);
			__m512 sin_local_tilt = _mm512_sqrt_ps(_mm512_div_ps(grade_clamped, denom));

			// max with K_tilt_minimum
			sin_local_tilt = _mm512_max_ps(sin_local_tilt, k_tilt_min);

			// ---- velocity magnitude (assume 2 components: vx, vy) ----
			// Using the component-stride pattern you gave for flux/velocity:
			// vx = vel_base + idx + velCompStride * 0
			// vy = vel_base + idx + velCompStride * 1
			__m512 vx = _mm512_loadu_ps(vel_base + idx + velCompStride * 0);
			__m512 vy = _mm512_loadu_ps(vel_base + idx + velCompStride * 1);
			// velLen = sqrt(vx*vx + vy*vy)
			__m512 velLen = _mm512_sqrt_ps(_mm512_fmadd_ps(vx, vx, _mm512_mul_ps(vy, vy)));

			// ---- capacity = K_sediment_capacity * velLen * sin_local_tilt * min(1, water_height) ----
			__m512 wh = _mm512_loadu_ps(&water_ptr[idx]);
			__m512 min1wh = _mm512_min_ps(one_f, wh);
			__m512 capacity = _mm512_mul_ps(k_sed_cap, _mm512_mul_ps(velLen, _mm512_mul_ps(sin_local_tilt, min1wh)));

			// ---- perlin factor & depth factor ----
			__m512 perlinF = _mm512_loadu_ps(&perlin_ptr[idx]);

			// depthDiff = originalHeight[idx] - terrain_height[idx]
			__m512 origH = _mm512_loadu_ps(&origH_ptr[idx]);
			__m512 terr = _mm512_loadu_ps(&terrain_ptr[idx]);
			__m512 depthDiff = _mm512_sub_ps(origH, terr);

			// depthFactor = 1 / (1 + depthDiff)
			__mmask16 maskNegDepth = _mm512_cmp_ps_mask(depthDiff, _mm512_set1_ps(0.0f), _CMP_LT_OQ);
			__m512 depthFactor = _mm512_div_ps(one_f, _mm512_add_ps(one_f, depthDiff));
			// if depthDiff < 0 -> depthFactor = 1
			depthFactor = _mm512_mask_mov_ps(_mm512_set1_ps(1.0f), maskNegDepth, depthFactor);

			// ---- capacity vs sediment decision (mask) ----
			__m512 sedv = _mm512_loadu_ps(&sed_ptr[idx]);
			__mmask16 maskDissolve = _mm512_cmp_ps_mask(capacity, sedv, _CMP_GT_OQ); // 1 -> dissolve, 0 -> deposit

			// ---- compute shared abs diff = |capacity - sediment| ----
			__m512 diff_raw = _mm512_sub_ps(capacity, sedv);
			__m512 diff = mm512_abs_ps_safe(diff_raw);

			// ---- compute dSoil for both branches while sharing work ----
			// k_mix: choose coefficient per lane (k_s_dissolve for dissolve lanes, k_d_deposit for deposit lanes)
			__m512 k_mix = _mm512_mask_blend_ps(maskDissolve, k_d_depo, k_s_diss);

			// base dSoil = k_mix * diff
			__m512 dSoil = _mm512_mul_ps(k_mix, diff);

			// For dissolving lanes only: multiply by perlinFactor * depthFactor
			__m512 modFactor = _mm512_mul_ps(perlinF, depthFactor);
			// multiply only where maskDissolve==1
			dSoil = _mm512_mask_mul_ps(dSoil, maskDissolve, dSoil, modFactor);

			// clamp to pMaxDissolve for all lanes
			dSoil = _mm512_min_ps(dSoil, maxD_vec);

			// For dissolve lanes only: further clamp to terrain height (cannot dissolve more than existing height)
			__m512 dSoil_clamped = _mm512_mask_min_ps(dSoil, maskDissolve, dSoil, terr);

			// For deposit lanes dSoil_clamped equals dSoil (mask keeps value)
			// Now dSoil_clamped contains the actual delta magnitude for both branches:
			// - if maskDissolve == 1 : amount to remove from terrain (and add to sediment)
			// - if maskDissolve == 0 : amount to add to terrain (and remove from sediment)

			// ---- update terrain and sediment branchlessly ----
			// terrain: dissolve -> terr - dSoil_clamped (mask=1)
			//          deposit  -> terr + dSoil_clamped (mask=0)
			// We'll do two masked ops: first mask=1: terrNew = terr - dSoil_clamped; then mask=~mask: add for deposit lanes
			__m512 terrNew = _mm512_mask_sub_ps(terr, maskDissolve, terr, dSoil_clamped);
			terrNew = _mm512_mask_add_ps(terrNew, ~maskDissolve, terrNew, dSoil_clamped);

			// sediment: dissolve -> sed + dSoil_clamped (mask=1)
			//           deposit -> sed - dSoil_clamped (mask=0)
			__m512 sedNew = _mm512_mask_add_ps(sedv, maskDissolve, sedv, dSoil_clamped);
			sedNew = _mm512_mask_sub_ps(sedNew, ~maskDissolve, sedNew, dSoil_clamped);

			// store results
			_mm512_storeu_ps(&terrain_ptr[idx], terrNew);
			_mm512_storeu_ps(&sed_ptr[idx], sedNew);
		} // end vectorized x loop

		// scalar remainder for this row (handle up to w-2)
	} // end y loop
}
*/

void Euler::ero5_simd(EulerGround& g)
{
	ND_PROFILE_METHOD();

	const int w = g.width;
	const int h = g.height;

	const __m512 kdt = _mm512_set1_ps(K_dt);

	for (int y = 1; y < h - 1 && e_erosion; ++y)
	{
		const int yw = y * w;
		for (int x = 1; x < w - 1 - 16; x += 16)
		{
			const int idx = yw + x;

			__m512 velX = _mm512_loadu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 0);
			__m512 velY = _mm512_loadu_ps(((float*)g.velocity.data()) + idx + g.velocity.size() * 1);

			velX = _mm512_mul_ps(velX, kdt);
			velY = _mm512_mul_ps(velY, kdt);

			// target position = current position - velocity * dt
			__m512 fx = _mm512_sub_ps(_mm512_set1_ps((gfloat)x), velX);
			__m512 fy = _mm512_sub_ps(_mm512_set1_ps((gfloat)y), velY);

			__m512 sediment = ter::interpolate2D(g.sediment.data(), w, h, fx, fy);

			_mm512_storeu_ps(&g.sediment[idx], sediment);
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

	const __m512 evap = _mm512_set1_ps(evapFactor);
	const __m512 epsilon = _mm512_set1_ps(evaporationEpsilon);

	for (int y = 1; y < h - 1 && e_evaporation; ++y)
	{
		const int yw = y * w;
		for (int x = 1; x < w - 1 - 16; x += 16)
		{
			const int idx = yw + x;

			// waterHeight *= evapFactor
			__m512 waterHeight = _mm512_loadu_ps(&g.water_height[idx]);
			waterHeight = _mm512_mul_ps(waterHeight, evap);
			// clamp to 0 if below epsilon
			__mmask16 mask = _mm512_cmp_ps_mask(waterHeight, epsilon, _CMP_LT_OQ);
			waterHeight = _mm512_mask_mov_ps(waterHeight, mask, _mm512_setzero_ps());

			// store
			_mm512_storeu_ps(&g.water_height[idx], waterHeight);
		}
	}
}

void Euler::ero7_simd(EulerGround& g)
{
	ND_PROFILE_METHOD();

	const int w = g.width;
	const int h = g.height;

	const gfloat multiplier = K_landSlideSpeed * K_dt;
	const __m512 k_mult = _mm512_set1_ps(multiplier);
	const __m512 zero = _mm512_setzero_ps();
	const __m512 landslideCutoff = _mm512_set1_ps(K_landSlideCutoffAngle);
	__mmask16 interleave = 0b0101010101010101;

	static bool interLeaveFlag = false;
	interLeaveFlag = !interLeaveFlag;
	if (interLeaveFlag)
		interleave = ~interleave;


	for (int y = 1; y < h - 1 && e_landslide; ++y)
	{
		const int yw = y * w;

		for (int x = 1; x < w - 1 - 16; x += 16)
		{
			const int idx = yw + x;

			__m512 height = _mm512_loadu_ps(&g.terrain_height[idx]);
			__m512 heightE = _mm512_loadu_ps(&g.terrain_height[idx + 1]); // east neighbor
			__m512 heightS = _mm512_loadu_ps(&g.terrain_height[idx + w]); // south neighbor
			__m512 deltaE = _mm512_sub_ps(heightE, height);
			__m512 deltaS = _mm512_sub_ps(heightS, height);

			__m512 takeNE = _mm512_mul_ps(deltaE, k_mult);
			__m512 takeNS = _mm512_mul_ps(deltaS, k_mult);

			__mmask16 signsE = _mm512_cmp_ps_mask(takeNE, zero, _CMP_LT_OQ);
			__mmask16 signsS = _mm512_cmp_ps_mask(takeNS, zero, _CMP_LT_OQ);

			takeNE = _mm512_max_ps(zero, _mm512_sub_ps(_mm512_abs_ps(takeNE), landslideCutoff));
			takeNS = _mm512_max_ps(zero, _mm512_sub_ps(_mm512_abs_ps(takeNS), landslideCutoff));


			takeNE = _mm512_mask_sub_ps(takeNE, signsE, zero, takeNE);
			takeNS = _mm512_mask_sub_ps(takeNS, signsS, zero, takeNS);


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
