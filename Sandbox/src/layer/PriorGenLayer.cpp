#include "ndpch.h"
#include "PriorGenLayer.h"
#include "core/imgui_utils.h"
#include "graphics/Effect.h"



static uint64_t s_seed;
static double s_time_speed=0.1;
static int s_width=720, s_height=480;
PriorGenLayer::PriorGenLayer():m_gen("/priorgen/")
{
	m_gen.gen(s_seed, s_height - 40, s_width, s_height);
	m_tex = m_gen.buildTexture();
}

void PriorGenLayer::onAttach()
{
}
static void runInner()
{

}


void PriorGenLayer::onUpdate()
{

	static int divider = 1;
	if (divider-- == 0) {
		divider = 30;
		m_gen.m_time += s_time_speed;
		m_gen.genLayer0();
		m_gen.updateTexture(m_tex);

	}
}

void PriorGenLayer::onRender()
{
	Effect::renderToCurrentFBO(m_tex);
}



void PriorGenLayer::onImGuiRender()
{
	static bool show = false;
	if (!ImGui::Begin("PriorGen", &show))
	{
		ImGui::End();
		return;
	}

	ImGui::InputInt("seed", ((int*)&s_seed));

	ImGui::InputInt("width", &s_width);
	ImGui::InputInt("height", &s_height);
	if (ImGui::Button("Update"))
	{
		m_gen.gen(s_seed,s_height-40 , s_width, s_height);
		delete m_tex;
		m_tex = m_gen.buildTexture();
	}

	ImGui::InputDouble("noise scale x", &m_gen.m_noise_scale_factor_x);
	ImGui::InputDouble("noise scale y", &m_gen.m_noise_scale_factor_y);
	ImGui::SliderDouble("noise cuttof", &m_gen.m_noise_cutoff,-1,1);
	ImGui::Separator();
	ImGui::SliderDouble("ore start", &m_gen.m_ore_start, 0, s_height);
	ImGui::SliderDouble("ore start full", &m_gen.m_ore_start_full, 0, s_height);
	ImGui::SliderDouble("ore stop partial", &m_gen.m_ore_end_partial, 0, s_height);
	ImGui::SliderDouble("ore stop", &m_gen.m_ore_end, 0, s_height);
	ImGui::SliderDouble("ore cluster size", &m_gen.m_ore_clusterSize, 0, 2);
	//ImGui::SliderDouble("ore cluster number", &m_gen.m_ore_clusterNumber, -5, 10);
	ImGui::SliderDouble("ore scale", &m_gen.m_orescale, 0, 10);
	ImGui::SliderDoubles("ore magn: ", m_gen.m_ore_octave_magnitudes, 5, 0, 5);

	
	ImGui::End();
}
