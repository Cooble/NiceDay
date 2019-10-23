#pragma once
#include <imgui.h>

namespace ImGui {
	// Plot value over time
	// Pass FLT_MAX value to draw without adding a new value
	void PlotVar(const char* label, float value, float scale_min = FLT_MAX, float scale_max = FLT_MAX, size_t buffer_size = 120);

	void PlotVar(const char* label, float value,bool maxOrMin, int resetTime=120,float scale_min = FLT_MAX, float scale_max = FLT_MAX, size_t buffer_size = 120);

	// Call this periodically to discard old/unused data
	void PlotVarFlushOldEntries();
	
	bool SliderDouble(const char* title, double* v, float min, float max, const char* format = "%.3f", float power = 1.0f);
	inline bool SliderDoubles(const char* title, double* vArray,int arraySize, float min, float max, const char* format = "%.3f", float power = 1.0f)
	{
		bool out = false;
		for (int i = 0; i < arraySize; ++i)
		{
			std::string t = title;
			t += std::to_string(i);
			out|=SliderDouble(t.c_str(), &vArray[i], min, max, format, power);
		}
		return out;
	}
}