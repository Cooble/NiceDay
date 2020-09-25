#pragma once
#include <imgui.h>

namespace ImGui {
	// Plot value over time
	// Pass FLT_MAX value to draw without adding a new value
	void PlotVar(const char* label, float value, float scale_min = FLT_MAX, float scale_max = FLT_MAX, size_t buffer_size = 120);

	void PlotVar(const char* label, float value, bool maxOrMin, int resetTime = 120, float scale_min = FLT_MAX, float scale_max = FLT_MAX, size_t buffer_size = 120);

	// Call this periodically to discard old/unused data
	void PlotVarFlushOldEntries();

	bool SliderDouble(const char* title, double* v, float min, float max, const char* format = "%.3f", float power = 1.0f);

	// converts v from radians to degrees
	inline bool InputFloatDegrees(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0)
	{
		float vv = glm::degrees(*v);
		if (InputFloat(label, &vv, step, step_fast, format, flags))
			*v = glm::radians(vv);
	}
	inline bool InputFloat2Degrees(const char* label, float v[2], const char* format = "%.3f", ImGuiInputTextFlags flags = 0)
	{
		auto vv = glm::degrees(*(glm::vec2*)v);
		if (InputFloat2(label, glm::value_ptr(vv), format, flags))
			*v = *glm::value_ptr(glm::radians(vv));
	}
	inline bool InputFloat3Degrees(const char* label, float v[2], const char* format = "%.3f", ImGuiInputTextFlags flags = 0)
	{
		auto vv = glm::degrees(*(glm::vec3*)v);
		if (InputFloat3(label, glm::value_ptr(vv), format, flags))
			*v = *glm::value_ptr(glm::radians(vv));
	}
	inline bool InputFloat4Degrees(const char* label, float v[2], const char* format = "%.3f", ImGuiInputTextFlags flags = 0)
	{
		auto vv = glm::degrees(*(glm::vec4*)v);
		if (InputFloat4(label, glm::value_ptr(vv), format, flags))
			*v = *glm::value_ptr(glm::radians(vv));
	}

	inline bool SliderDoubles(const char* title, double* vArray, int arraySize, float min, float max, const char* format = "%.3f", float power = 1.0f)
	{
		bool out = false;
		for (int i = 0; i < arraySize; ++i)
		{
			std::string t = title;
			t += std::to_string(i);
			out |= SliderDouble(t.c_str(), &vArray[i], min, max, format, power);
		}
		return out;
	}
}