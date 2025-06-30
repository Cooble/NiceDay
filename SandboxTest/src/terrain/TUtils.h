#pragma once
#include <utility>
#include <vector>
#include <glm/common.hpp>
#include <glm/gtc/noise.hpp>
#include <imgui.h>

#include "types.h"

namespace ter
{
	inline gfloat interpolate2D(const std::vector<gfloat>& data, int width, int height, gfloat x, gfloat y)
	{
		int x0 = (int)x;
		int y0 = (int)y;
		int x1 = x0 + 1;
		int y1 = y0 + 1;

		x0 = glm::clamp(x0, 0, width - 1);
		y0 = glm::clamp(y0, 0, height - 1);
		x1 = glm::clamp(x1, 0, width - 1);
		y1 = glm::clamp(y1, 0, height - 1);

		gfloat xLerp = x - x0;
		gfloat yLerp = y - y0;

		gfloat h00 = data[y0 * width + x0];
		gfloat h01 = data[y0 * width + x1];
		gfloat h10 = data[y1 * width + x0];
		gfloat h11 = data[y1 * width + x1];
		return h00 * (1 - xLerp) * (1 - yLerp) +
			h01 * (xLerp) * (1 - yLerp) +
			h10 * (yLerp) * (1 - xLerp) +
			h11 * (xLerp) * (yLerp);
	}

	inline gvec2 gradAt(std::vector<gfloat>& scalarField, int x, int y, int w)
	{
		auto x0 = glm::clamp(x, 0, w - 1);
		auto y0 = glm::clamp(y, 0, w - 1);
		auto x1 = glm::clamp(x + 1, 0, w - 1);
		auto y1 = glm::clamp(y + 1, 0, w - 1);
		auto gradX = scalarField[y0 * w + x1] - scalarField[y0 * w + x0];
		auto gradY = scalarField[y1 * w + x0] - scalarField[y0 * w + x0];
		return {gradX, gradY};
	}

	inline gvec2 interPol(gvec2 v00, gvec2 v01, gvec2 v10, gvec2 v11, gvec2 pos)
	{
		return v00 * (1 - pos.x) * (1 - pos.y) +
			v01 * (pos.x) * (1 - pos.y) +
			v10 * (pos.y) * (1 - pos.x) +
			v11 * (pos.x) * (pos.y);
	}





	static gfloat myPerlin(gvec2 uv)
	{
		// Use glm::perlin (which expects a vec2)
		auto out =
			glm::perlin(uv) +
			glm::perlin((uv - (gfloat)20) * (gfloat)2.f) * (gfloat)0.5f +
			glm::perlin((uv - (gfloat)1235.4) * (gfloat)4.f) * (gfloat)0.25f;

		// normalize to [-1, 1]
		out /= 1.75f;

		// normalize to [0, 1]
		out = (out + 1.f) / 2.f;

		return out;
	}

	inline void generate2DPerlin(std::vector<gfloat>& data, int width, int height)
	{
		for (int x = 0; x < width; x++)
			for (int y = 0; y < height; y++)
			{
				auto idx = y * width + x;
				data[idx] = myPerlin(gvec2((gfloat)x / width, (gfloat)y / height) * (gfloat)10);
			}
			
	}
	template <typename T>
	static bool SliderGFloat(const char* label, T* v, float min, float max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	{
		if constexpr (std::is_same_v<T, float>)
		{
			return ImGui::SliderFloat(label, v, min, max, format, flags);
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			return ImGui::SliderScalar(label, ImGuiDataType_Double, v, &min, &max, format, flags);
		}
		else static_assert(false, "Unsupported type for SliderGFloat");
	}
	template <typename T>
	static bool SliderGFloat2(const char* label, T* v, float min, float max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	{
		if constexpr (std::is_same_v<T, float>)
		{
			return ImGui::SliderFloat2(label, v, min, max, format, flags);
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			return ImGui::SliderScalarN(label, ImGuiDataType_Double, v, 2, &min, &max, format, flags);
		}
		else static_assert(false, "Unsupported type for SliderGFloat2");
	}

	template <typename T>
	static bool InputGFloat(const char* label, T* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f",
		ImGuiInputTextFlags flags = 0)
	{
		if constexpr (std::is_same_v<T, float>)
		{
			return ImGui::InputFloat(label, v, step, step_fast, format, flags);
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			return ImGui::InputDouble(label, v, step, step_fast, format, flags);
		}
		else static_assert(false, "Unsupported type for InputGFloat");
	}


	// Helper function to add a formatted row to the current ImGui table (assumes 2 columns)
	template<typename... Args>
	void ImGuiAddTableRow(const char* label, const char* format, Args&&... args)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label); // Use TextUnformatted for the label column
		ImGui::TableNextColumn();
		// Forward arguments to ImGui::Text, which handles printf-style formatting
		ImGui::Text(format, std::forward<Args>(args)...);
	}

}
