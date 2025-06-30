#pragma once
#include "ndpch.h"

#include <vector>

#include "BaseGround.h"
#include "types.h"


namespace nd
{
	class NBT;
}

// EulerGround is a specialized ground structure for the Euler simulation
struct EulerGround : BaseGround
{
	std::vector<gfloat> water_height;
	std::vector<gfloat> sediment;
	std::vector<gvec4> flux;
	std::vector<gvec2> velocity;

	void resize(int size)
	{
		BaseGround::resize(size);
		
		auto sq = width * width;
		water_height.resize(sq);
		sediment.resize(sq);
		flux.resize(sq);
		velocity.resize(sq);

		ZeroMemory(water_height.data(), water_height.size() * sizeof(decltype(water_height)::value_type));
		ZeroMemory(sediment.data(), sediment.size() * sizeof(decltype(sediment)::value_type));
		ZeroMemory(flux.data(), flux.size() * sizeof(decltype(flux)::value_type));
		ZeroMemory(velocity.data(), velocity.size() * sizeof(decltype(velocity)::value_type));
	}
};


struct Euler
{
	static constexpr gfloat pPipeArea = 0.6f;
	static constexpr gfloat pGravity = 9.81f;
	static constexpr gfloat pPipeLen = 1.f;
	static constexpr gfloat pLL = 1.f;
	static constexpr gfloat pMaxDissolve = 0.1;


	bool e_rain = false;
	bool e_flow = true;
	bool e_erosion = true;
	bool e_evaporation = true;
	bool e_landslide = true;

	int groundSize = 128;
	gfloat totalGround = 0;
	gfloat currentGround = 0;
	gfloat currentSediment = 0;
	gfloat maxSediment = 0;
	gfloat currentWater = 0;
	gfloat minTerrain = 0, maxTerrain = 0;


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


	std::vector<gfloat> perlinMap;
	std::vector<gfloat> originalHeight;
	std::vector<gfloat> sediment;

	// prepare special fields based on terrain_height
	void init(EulerGround& g);


	void step(EulerGround& g);


	void imguiRender();


	void save(nd::NBT& src);
	void load(nd::NBT& src);

	
};
