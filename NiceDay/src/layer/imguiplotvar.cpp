#include "ndpch.h"
#include "imguiplotvar.h"
#include <map>

namespace  ImGui
{
	struct PlotVarData
	{
		ImGuiID        ID;
		std::vector<float>  Data;
		int            DataInsertIdx;
		int            LastFrame;
		float			LastNormalVal;

		PlotVarData() : ID(0), DataInsertIdx(0), LastFrame(-1) {}
	};

	typedef std::map<ImGuiID, PlotVarData> PlotVarsMap;
	static PlotVarsMap	g_PlotVarsMap;

	// Plot value over time
	// Call with 'value == FLT_MAX' to draw without adding new value to the buffer
	void PlotVar(const char* label, float value, float scale_min, float scale_max, size_t buffer_size)
	{
			
		assert(label);
		if (buffer_size == 0)
			buffer_size = 1000;

		ImGui::PushID(label);
		ImGuiID id = ImGui::GetID("");

		// Lookup O(log N)
		PlotVarData& pvd = g_PlotVarsMap[id];
		if (value != FLT_MAX)
			pvd.LastNormalVal = value;


		// Setup
		if (pvd.Data.capacity() != buffer_size)
		{
			pvd.Data.resize(buffer_size);
			memset(&pvd.Data[0], 0, sizeof(float) * buffer_size);
			pvd.DataInsertIdx = 0;
			pvd.LastFrame = -1;
		}

		// Insert (avoid unnecessary modulo operator)
		if (pvd.DataInsertIdx == buffer_size)
			pvd.DataInsertIdx = 0;
		int display_idx = pvd.DataInsertIdx;
		if (value != FLT_MAX)
			pvd.Data[pvd.DataInsertIdx++] = value;

		// Draw
		int current_frame = ImGui::GetFrameCount();
		if (pvd.LastFrame != current_frame)
		{
			ImGui::PlotLines("##plot", &pvd.Data[0], buffer_size, pvd.DataInsertIdx, NULL, scale_min, scale_max, ImVec2(0, 40));
			ImGui::SameLine();
			ImGui::Text("%s\n%-3.0f", label, pvd.LastNormalVal);	// Display last value in buffer
			pvd.LastFrame = current_frame;
		}

		ImGui::PopID();
	}
	int max(int a ,int b)
	{
		if (a > b)
			return a;
		return b;
	}

	void PlotVarFlushOldEntries()
	{
		int current_frame = ImGui::GetFrameCount();
		for (auto it = g_PlotVarsMap.begin(); it != g_PlotVarsMap.end(); )
		{
			PlotVarData& pvd = it->second;
			if (pvd.LastFrame < current_frame - max(400, (int)pvd.Data.size()))
				it = g_PlotVarsMap.erase(it);
			else
				++it;
		}
	}
}
