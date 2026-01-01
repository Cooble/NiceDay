#include "ndpch.h"
#include "EulerSim.h"
#include <ImGuiFileDialog.h>

#include "TerrainLayer.h"
#include "TUtils.h"
#include "core/NBT.h"

#include <immintrin.h>

#include <algorithm>

#include "ero.h"
#define ERO_PARALLEL_ENABLE 1
#include "ero.h"


/*#include "ero_simd.h"
#define EROSIMD_PARALLEL_ENABLE 1
#include "ero_simd.h"*/

// 1. Primitive / Vanilla version (No Defines)
#include "ero_new.h" 

// 2. Parallel STL version
#define EROSIMD_PARALLEL_ENABLE
#include "ero_new.h"
#undef EROSIMD_PARALLEL_ENABLE

// 3. OpenMP version
#define EROSIMD_OMP_ENABLE
#include "ero_new.h"
#undef EROSIMD_OMP_ENABLE



#include "cl_context.h"

#undef ND_PROFILE_METHOD
#define ND_PROFILE_METHOD()


//#define USE_PARALLEL


#ifdef USE_PARALLEL
#define PARALLELIZE_LOOP _Pragma("omp parallel for schedule(static)")
#else
#define PARALLELIZE_LOOP
#endif


#ifdef USE_PARALLEL
#define PARALLELIZE_LOOP_SIMD _Pragma("omp parallel for schedule(static)")
#else
#define PARALLELIZE_LOOP_SIMD
#endif



#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <algorithm>


using namespace nd;

void Euler::init(EulerGround& g, EulerSettings* s,bool generatePerlin)
{
	this->s = s;
	perlinMap.resize(g.width * g.width);
	sedimentDontUse = g.sediment;

	g.new_sediment = g.sediment;
	g.original_height = g.terrain_height;
	originalHeight = g.terrain_height;
	g.new_terrain_height = g.terrain_height;
	g.total_height = g.terrain_height;

	if (generatePerlin)
		ter::generate2DPerlin(REINTERPRET_AS(std::vector<gfloat>, perlinMap), g.width, g.height);
	g.perlin_map = perlinMap;

	ZeroMemory(g.water_height.data(), g.water_height.size() * sizeof(decltype(g.water_height)::value_type));

	if (sim_type == OPENCL) 
		cl->init(g, *s);
	refreshParams(g, *s);

}

void Euler::refreshParams(EulerGround& g,EulerSettings& s)
{
	if (sim_type == OPENCL && settings_dirty) {
	//if (sim_type == OPENCL) {
		cl->upload_params(g, s);
		settings_dirty = false;
	}
}

static uint64_t sMeasureFunctionTime = 1;

void measureFunction(std::function<void(EulerGround&,Euler::EulerSettings*)> a1, std::function<void(EulerGround&, Euler::EulerSettings*)> a2, EulerGround& g,Euler::EulerSettings* s)
{
	uint64_t micros1, micros2;
	{
		TimerStaper p("");
		for (int i = 0; i < sMeasureFunctionTime; ++i)
			a1(g,s);
		micros1 = p.getUS();
	}
	{
		TimerStaper p("");
		for (int i = 0; i < sMeasureFunctionTime; ++i)
			a2(g,s);
		micros2 = p.getUS();
	}
	ND_BUG("SIMD took {} us, BASIC took {} us, SpeedUp of {}", micros1 / sMeasureFunctionTime,
	       micros2 / sMeasureFunctionTime, (gfloat)micros2 / (gfloat)micros1);
}

#define measureFuncFixed(name) measureFunction(&name##_simd, &(name), g,s )

void Euler::step(EulerGround& g)
{
	if (g.height * g.height != sedimentDontUse.size())
		init(g,this->s);

	switch (sim_type)
	{
	case CPU_BASIC:
		ero1(g, s);
		ero2(g, s);
		ero3(g, s);
		ero4(g, s);
		ero5(g, s);
		ero7(g, s);
		return;

	case CPU_PARALLEL:
		EroParallel::ero1(g, s);
		EroParallel::ero2(g, s);
		EroParallel::ero3(g, s);
		EroParallel::ero4(g, s);
		EroParallel::ero5(g, s);
		EroParallel::ero7(g, s);
		return;

	case CPU_SIMD:
		ero1_simd(g, s);
		ero2_simd(g, s);
		ero2_simd_fix_borders(g);
		ero3_simd(g, s);
		ero3_simd_fix_borders(g);
		ero4_simd(g, s);
		ero5_simd(g, s);
		ero7_simd(g, s);
		return;

	case SIMD_PARALLEL:
		EroParallel::ero1_simd(g, s);
		EroParallel::ero2_simd(g, s);
		ero2_simd_fix_borders(g);
		EroParallel::ero3_simd(g, s);
		ero3_simd_fix_borders(g);
		EroParallel::ero4_simd(g, s);
		EroParallel::ero5_simd(g, s);
		EroParallel::ero7_simd(g, s);
		return;

	case OPENCL:
		//cl->upload_all(g);
		//cl->upload_params(g, *s);
		cl->step(*s);
		return;

	case SIMD_PARALLEL_OMP:
		EroOmp::ero1_simd(g, s);
		EroOmp::ero2_simd(g, s);
		ero2_simd_fix_borders(g);
		EroOmp::ero3_simd(g, s);
		ero3_simd_fix_borders(g);
		EroOmp::ero4_simd(g, s);
		EroOmp::ero5_simd(g, s);
		EroOmp::ero7_simd(g, s);
		return;
	}


	static bool first = true;
	if (first)
	{
		first = false;
		sMeasureFunctionTime = 1; //warmup
	}
	else
		sMeasureFunctionTime = 50;


	ND_BUG("======================Measuring Euler Steps");
	measureFuncFixed(ero1);
	measureFuncFixed(ero2);
	measureFuncFixed(ero3);
	measureFuncFixed(ero4);
	measureFuncFixed(ero5);
	measureFuncFixed(ero7);
	ND_BUG("======================Done");
}

void Euler::stepRender(EulerGround& g)
{
	if (sim_type == OPENCL)
		cl->download_graphics(g);
}

void Euler::imguiRender()
{
	EulerSettings pastSettings = *s;

	using namespace ter;
	ImGui::Checkbox("Rain", &s->e_rain);
	ImGui::Checkbox("Flow", &s->e_flow);
	ImGui::Checkbox("Erosion", &s->e_erosion);
	ImGui::Checkbox("Evaporation", &s->e_evaporation);
	ImGui::Checkbox("Landslide", &s->e_landslide);

	ImGui::SeparatorText("Euler Settings");
	ImGui::PushID(this);


	InputGFloat("Gravity", &s->K_g, 0, 0, "%.6f");
	SliderGFloat("Dt", &s->K_dt, 0, (gfloat)0.1, "%.6f");
	SliderGFloat("Rain", &s->K_rain, 0, 10, "%.3f");
	SliderGFloat("Sediment Capacity", &s->K_sediment_capacity, 0, (gfloat)0.5, "%.6f");
	SliderGFloat("Dissolving", &s->K_s_dissolving, 0, (gfloat)0.5, "%.6f");
	SliderGFloat("Depositing", &s->K_d_depositing, 0, (gfloat)0.5, "%.6f");
	SliderGFloat("Evaporation", &s->K_evaporation, 0, (gfloat)0.5, "%.6f");

	SliderGFloat("Tilt Minimum", &s->K_tilt_minimum, 0, 10, "%.3f");
	SliderGFloat("Land Slide Speed", &s->K_landSlideSpeed, 0, 1, "%.3f");
	SliderGFloat("Land Slide Cutoff Angle", &s->K_landSlideCutoffAngle, 0, 1, "%.3f");

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

	// check if change happened
	if (!(pastSettings==*s))
		settings_dirty = true;
}

void Euler::save(nd::NBT& src)
{
	NBT_SAVE(src, s->K_dt);
	NBT_SAVE(src, s->K_rain);
	NBT_SAVE(src, s->K_g);
	NBT_SAVE(src, s->K_sediment_capacity);
	NBT_SAVE(src, s->K_s_dissolving);
	NBT_SAVE(src, s->K_d_depositing);
	NBT_SAVE(src, s->K_evaporation);
	NBT_SAVE(src, s->K_tilt_minimum);
	NBT_SAVE(src, s->K_landSlideSpeed);
	NBT_SAVE(src, s->K_landSlideCutoffAngle);
}

void Euler::load(nd::NBT& src)
{
	NBT_LOAD(src, s->K_dt);
	NBT_LOAD(src, s->K_rain);
	NBT_LOAD(src, s->K_g);
	NBT_LOAD(src, s->K_sediment_capacity);
	NBT_LOAD(src, s->K_s_dissolving);
	NBT_LOAD(src, s->K_d_depositing);
	NBT_LOAD(src, s->K_evaporation);
	NBT_LOAD(src, s->K_tilt_minimum);
	NBT_LOAD(src, s->K_landSlideSpeed);
	NBT_LOAD(src, s->K_landSlideCutoffAngle);
}

Euler::Euler() :cl(new EroCLContext())
{
}

Euler::~Euler()
{
	delete cl;
}

void Euler::ero3_simd_fix_borders(EulerGround& g)
{
	auto w = g.width;
	auto h = g.height;

	// now we nullify velocity on borders if needed
	for (int y = 1; y < h - 1; y++)
	{
		int row = y * w;
		auto& velLLeft = *(((float*)g.velocity.data()) + (row + 1) + g.velocity.size() * 0);
		auto& velLRight = *(((float*)g.velocity.data()) + (row + w - 2) + g.velocity.size() * 0);

		velLLeft = std::max(0.f, velLLeft);
		velLRight = std::min(0.f, velLRight);
	}
	// top and bottom
	for (int x = 0; x < w; x++)
	{
		auto& velTop = *(((float*)g.velocity.data()) + (x + w) + g.velocity.size() * 1);
		auto& velBottom = *(((float*)g.velocity.data()) + ((h - 2) * w + x) + g.velocity.size() * 1);
		velTop = std::max(0.f, velTop);
		velBottom = std::min(0.f, velBottom);
	}
}

void Euler::ero2_simd_fix_borders(EulerGround& g)
{
	auto w = g.width;
	auto h = g.height;

	// nullify borders
	// now we need to set outfluxes to zero on borders
	for (int y = 1; y < h - 1; y++)
	{
		int row = y * w;
		*((float*)g.flux.data() + (row + 1) + g.flux.size() * 0) = 0.f;
		*((float*)g.flux.data() + (row + w - 1 - 1) + g.flux.size() * 1) = 0.f;
	}
	// top and bottom
	ZeroMemory(((float*)g.flux.data()) + g.flux.size() * 2 + w, w * sizeof(float));
	ZeroMemory(((float*)g.flux.data()) + g.flux.size() * 3 + (h - 2) * w, w * sizeof(float));
}

void Euler::ero3_fix_borders(EulerGround& g)
{
	auto w = g.width;
	auto h = g.height;

	// now we nullify velocity on borders if needed
	for (int y = 1; y < h - 1; y++)
	{
		int row = y * w;
		auto& velLeft = g.velocity[row + 1].x;
		auto& velRight = g.velocity[row + w - 2].x;
		velLeft = glm::max((gfloat)0, velLeft);
		velRight = glm::min((gfloat)0, velRight);
	}
	// top and bottom
	for (int x = 0; x < w; x++)
	{
		auto& velTop = g.velocity[w + x].y;
		auto& velBottom = g.velocity[(h - 2) * w + x].y;
		velTop = glm::max((gfloat)0, velTop);
		velBottom = glm::min((gfloat)0, velBottom);
	}

}

void Euler::ero2_fix_borders(EulerGround& g)
{
	auto w = g.width;
	auto h = g.height;


	// nullify borders
	// now we need to set outfluxes to zero on borders
	for (int y = 1; y < h - 1; y++)
	{
		int row = y * w;
		g.flux[row+1].x =  0.f;
		g.flux[row + w - 1 - 1].y = 0.f;
		
	}
	// top and bottom
	for (int x = 0; x < w; x++)
	{
		g.flux[w + x].z = 0.f;
		g.flux[(h - 2) * w + x].w = 0.f;
	}
}
