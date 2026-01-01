#pragma once
#include "ndpch.h"

#include <vector>

#include "BaseGround.h"
#include "types.h"


class EroCLContext;

namespace nd
{
	class NBT;
}

// EulerGround is a specialized ground structure for the Euler simulation
struct EulerGround : BaseGround
{
	AVector<gfloat> water_height;
	AVector<gfloat> sediment;
	AVector<gvec4> flux;
	AVector<gvec2> velocity;

	AVector<gfloat> new_terrain_height;
	AVector<gfloat> new_sediment;
	AVector<gfloat> perlin_map;
	AVector<gfloat> original_height;
	AVector<gfloat> total_height;

	ParallelChunks pc;


	void resize(int size)
	{
		BaseGround::resize(size);
		
		auto sq = width * width;
		water_height.resize(sq);
		sediment.resize(sq);
		flux.resize(sq);
		velocity.resize(sq);
		new_terrain_height.resize(sq);
		new_sediment.resize(sq);
		perlin_map.resize(sq);
		original_height.resize(sq);
		total_height.resize(sq);

		pc.initialize(1, height-1);


		ZeroMemory(water_height.data(), water_height.size() * sizeof(decltype(water_height)::value_type));
		ZeroMemory(sediment.data(), sediment.size() * sizeof(decltype(sediment)::value_type));
		ZeroMemory(flux.data(), flux.size() * sizeof(decltype(flux)::value_type));
		ZeroMemory(velocity.data(), velocity.size() * sizeof(decltype(velocity)::value_type));
		ZeroMemory(new_terrain_height.data(), new_terrain_height.size() * sizeof(decltype(new_terrain_height)::value_type));
		ZeroMemory(new_sediment.data(), new_sediment.size() * sizeof(decltype(new_sediment)::value_type));

	}
};


struct Euler
{
	static constexpr gfloat pPipeArea = 0.6f;
	static constexpr gfloat pGravity = 9.81f;
	static constexpr gfloat pPipeLen = 1.f;
	static constexpr gfloat pLL = 1.f;
	static constexpr gfloat pMaxDissolve = 0.1f;

	enum SimType: int
	{
		CPU_BASIC,
		CPU_SIMD,
		OPENCL,
		CPU_PARALLEL,
		SIMD_PARALLEL,
		SIMD_PARALLEL_OMP
	} sim_type = CPU_BASIC;
	constexpr static const char* sim_type_names[] = {
		"CPU_BASIC",
		"CPU_SIMD",
		"OPENCL",
		"CPU_PARALLEL",
		"SIMD_PARALLEL",
		"SIMD_PARALLEL_OMP"
	};

	struct EulerSettings
	{
		bool e_rain = true;
		bool e_flow = true;
		bool e_erosion = true;
		bool e_evaporation = true;
		bool e_landslide = true;

		gfloat K_rain = 0.01f;
		gfloat K_g = 9.81f;
		// Sediment Capacity
		gfloat K_sediment_capacity = 0.05f;
		// Dissolving constant 
		gfloat K_s_dissolving = 0.1f;
		// Depositing constant
		gfloat K_d_depositing = 0.03f;
		// Evaporation constant
		gfloat K_evaporation = 0.03f;
		gfloat K_tilt_minimum = 0.15f;
		gfloat K_landSlideSpeed = 20.5f;
		gfloat K_landSlideCutoffAngle = 0.80f;
		gfloat K_dt = 0.004f;

		bool operator==(const EulerSettings& s) const
		{
			return e_rain == s.e_rain &&
				e_flow == s.e_flow &&
				e_erosion == s.e_erosion &&
				e_evaporation == s.e_evaporation &&
				e_landslide == s.e_landslide &&
				K_rain == s.K_rain &&
				K_g == s.K_g &&
				K_sediment_capacity == s.K_sediment_capacity &&
				K_s_dissolving == s.K_s_dissolving &&
				K_d_depositing == s.K_d_depositing &&
				K_evaporation == s.K_evaporation &&
				K_tilt_minimum == s.K_tilt_minimum &&
				K_landSlideSpeed == s.K_landSlideSpeed &&
				K_landSlideCutoffAngle == s.K_landSlideCutoffAngle &&
				K_dt == s.K_dt;
		}
	};
	EulerSettings* s;
	EroCLContext* cl = nullptr;

	
	int groundSize = 128;
	gfloat totalGround = 0;
	gfloat currentGround = 0;
	gfloat currentSediment = 0;
	gfloat maxSediment = 0;
	gfloat currentWater = 0;
	gfloat minTerrain = 0, maxTerrain = 0;


	AVector<gfloat> perlinMap;
	AVector<gfloat> originalHeight;
	AVector<gfloat> sedimentDontUse;

	// prepare special fields based on terrain_height
	void init(EulerGround& g,EulerSettings * s,bool generatePerlin=true);

	void refreshParams(EulerGround& g, EulerSettings& s);
	void step(EulerGround& g);
	void stepRender(EulerGround& g);

	bool settings_dirty = true;

	void imguiRender();


	void save(nd::NBT& src);
	void load(nd::NBT& src);

	Euler();
	~Euler();

public:

	void ero3_fix_borders(EulerGround& g);
	void ero2_fix_borders(EulerGround& g);
	

	
};
