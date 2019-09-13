#include "ndpch.h"
#include  "imgui_utils.h"
#include "imgui.h"

namespace ImGui {
	bool SliderDouble(const char* title, double* v, float min, float max, const char* format, float power)
	{
		float f = (float)*v;
		bool s = ImGui::SliderFloat(title, &f, min, max, format, power);
		*v = (double)f;

		return s;
	}
}
