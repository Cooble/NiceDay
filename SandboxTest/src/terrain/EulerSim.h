#pragma once
#include "ndpch.h"

#include <vector>

#include "types.h"


struct Ground;

struct Euler
{
	static constexpr gfloat pPipeArea = 0.6f;
	static constexpr gfloat pGravity = 9.81f;
	static constexpr gfloat pPipeLen = 1.f;
	static constexpr gfloat pLL = 1.f;
	static constexpr gfloat pMaxDissolve = 0.1;


	bool e_rain = false;
	bool e_flow = false;
	bool e_erosion = false;
	bool e_evaporation = false;
	bool e_landslide = false;

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


	void init(Ground& g);


	void step(Ground& g);


	void imguiRender();
	
};
