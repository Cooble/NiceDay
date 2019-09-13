#pragma once
namespace ImGui {
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