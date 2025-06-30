#pragma once
#include <vector>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/noise.hpp>

#include "imgui.h"
#include "terrain/TUtils.h"
#include "terrain/types.h"

struct BaseGround
{
	std::vector<gfloat> terrain_height;
	int width, height;


	void resize(int size)
	{
		width = size;
		height = size;
		auto sq = width * height;
		terrain_height.resize(sq);
		ZeroMemory(terrain_height.data(), terrain_height.size() * sizeof(decltype(terrain_height)::value_type));
	}


	static void generateBasin(BaseGround& a, gfloat scale = 0.75, gfloat offset = 0.25)
	{
		for (int x = 0; x < a.width; x++)
		{
			for (int y = 0; y < a.height; y++)
			{
				gfloat xx = (gfloat)x / a.width;
				gfloat yy = (gfloat)y / a.height;

				auto delta = glm::max(glm::abs(xx - 0.5), glm::abs(yy - 0.5));

				gfloat d = delta * scale + offset;

				d *= a.width;
				a.terrain_height[x + y * a.width] = d;
			}
		}
	}

	static void generateFlat(BaseGround& a, gfloat height = 0.5)
	{
		a.terrain_height.assign(a.terrain_height.size(), height * a.width);
	}

	struct PerlinNoiseLayer
	{
		gfloat frequency;
		gfloat amplitude;
	};

	static void generatePerlinMultiOctave(BaseGround& a, const std::vector<PerlinNoiseLayer>& layers,
	                                      gfloat scale = 1.0f, const gvec2& uvOffset = gvec2(0.0f))
	{
		auto size = a.width;
		for (int y = 0; y < size; ++y)
		{
			for (int x = 0; x < size; ++x)
			{
				const gfloat u = static_cast<gfloat>(x) / static_cast<gfloat>(size);
				const gfloat v = static_cast<gfloat>(y) / static_cast<gfloat>(size);

				gfloat value = 0.0f;
				gfloat norm = 0.0f; // Sum of amplitudes (for normalization)

				for (const auto& layer : layers)
				{
					const gfloat freq = layer.frequency * scale;
					const gfloat amp = layer.amplitude;

					const glm::vec2 p = (gvec2(u, v) + uvOffset) * freq;
					value += glm::perlin(p) * amp; // glm::perlin returns roughly [‑1,1]
					norm += amp;
				}

				// Map from [‑norm, norm] → [0,1]
				value = (value / norm) * 0.5f + 0.5f;
				a.terrain_height[x + y * size] = value *a.width;
			}
		}
	}

	static void generateGroundSine(BaseGround& a, gfloat amplitude = 1,gfloat freq = 1,gfloat phase = 0)
	{
		for (int x = 0; x < a.width; x++)
		{
			for (int y = 0; y < a.height; y++)
			{
				gfloat xx = (gfloat)x / a.width;
				gfloat yy = (gfloat)y / a.height;
				xx -= 0.5f;
				xx *= 2.f;
				yy -= 0.5f;
				yy *= 2.f;
				
				gfloat d = (glm::sin(glm::sqrt(xx * xx + yy * yy) * 2 * glm::pi<gfloat>() * freq + glm::radians(phase)) + 1) / 2;

				d /= 2;
				//mesh.map(x, y) = (x + y) / (gfloat)mesh.map.width / 2.f;
				d *= 0.9f;
				d += 0.1f * glm::max(x, y) / a.width;

				d *= a.width;

				a.terrain_height[x + y * a.width] = d* amplitude;
			}
		}
	}
};

namespace BaseGroundImgui
{
	namespace Perlin
	{
		/*static std::vector<BaseGround::PerlinNoiseLayer> uiLayers = {
			{1.0f, 1.0f}, // Base shape
			{2.0f, 0.5f}, // Medium detail
			{4.0f, 0.25f}, // Fine detail
			{8.0f, 0.125f} // Very fine detail
		};*/
		static std::vector<BaseGround::PerlinNoiseLayer> uiLayers = {
			{0.53f, 1.0f}, 
			{1.51f, 0.85f}, 
			{6.77f, 0.238f},
		};
		//static float uiScale = 1.66f;
		static float uiScale = 2.8f;
		static glm::vec2 uiOffset = glm::vec2(0.11f,0.f);


		// return true if change
		static bool Show()
		{
			if (!ImGui::CollapsingHeader("Perlin"))
				return false;

			bool changed = false;
			changed |= ter::SliderGFloat("Global Scale", &uiScale, 0.01f, 5.0f, "%.2f");
			changed |= ter::SliderGFloat2("Offset (UV)", &uiOffset[0], -10.0f, 10.0f, "%.2f");

			ImGui::Separator();
			ImGui::Text("Octaves:");

			for (int i = 0; i < uiLayers.size(); ++i)
			{
				ImGui::PushID(i);
				changed |= ter::SliderGFloat("Frequency", &uiLayers[i].frequency, 0.1f, 30.0f, "%.2f");

				changed |= ter::SliderGFloat("Amplitude", &uiLayers[i].amplitude, 0.0f, 1.0f, "%.3f");
				ImGui::SameLine();
				if (ImGui::SmallButton("X"))
				{
					uiLayers.erase(uiLayers.begin() + i);
					changed = true;
					ImGui::PopID();
					// Avoid "skip" issues after erase
					continue;
				}
				ImGui::Separator();
				ImGui::PopID();
			}

			if (ImGui::Button("Add Layer"))
			{
				uiLayers.push_back({1.0f, 0.5f});
				changed = true;
			}
			return changed;
		}
	}

	namespace Basin
	{
		static gfloat uiScale = 0.75f;
		static gfloat uiOffset = 0.25f;

		static bool Show()
		{
			if (!ImGui::CollapsingHeader("Basin"))
				return false;

			bool changed = false;
			changed |= ter::SliderGFloat("Scale", &uiScale, 0.01f, 2.0f, "%.2f");
			changed |= ter::SliderGFloat("Offset", &uiOffset, 0.0f, 1.0f, "%.2f");
			return changed;
		}
	}

	namespace Flat
	{
		static gfloat uiHeight = 0.5f;

		static bool Show()
		{
			if (!ImGui::CollapsingHeader("Flat"))
				return false;

			bool changed = false;
			changed |= ter::SliderGFloat("Height", &uiHeight, 0.0f, 1.0f, "%.2f");
			return changed;
		}
	}

	namespace Sine
	{
		static gfloat uiFreq = 1.0f;
		static gfloat uiAmplitude = 1.0f;
		static gfloat uiPhase = 0;

		static bool Show()
		{
			if (!ImGui::CollapsingHeader("Sine"))
				return false;

			bool changed = false;
			changed |= ter::SliderGFloat("Frequency", &uiFreq, 0.01f, 5.0f, "%.2f");
			changed |= ter::SliderGFloat("Amplitude", &uiAmplitude, 0, 1, "%.2f");
			changed |= ter::SliderGFloat("Phase", &uiPhase, 0, 360, "%.0f");

			return changed;
		}
	}
}
