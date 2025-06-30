#include "EulerSim.h"

#include <ImGuiFileDialog.h>
#include <glm/gtc/noise.hpp>

#include "TerrainLayer.h"
#include "TUtils.h"
#include "core/NBT.h"


void Euler::init(EulerGround& g)
{
	perlinMap.resize(g.width * g.width);
	sediment = g.sediment;
	originalHeight = g.terrain_height;
	ter::generate2DPerlin(perlinMap, g.width, g.height);
	ZeroMemory(g.water_height.data(), g.water_height.size() * sizeof(decltype(g.water_height)::value_type));
}

void Euler::step(EulerGround& g)
{
	if (g.height * g.height != sediment.size())
		init(g);

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
			g.water_height[x + y * w] +=K_dt * increase;
		}

	auto totalHeight = g.terrain_height;
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


			g.sediment[idx] = ter::interpolate2D(g.sediment, w, h, fx, fy);
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
