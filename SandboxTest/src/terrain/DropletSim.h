#pragma once
#include "ndpch.h"
#include "types.h"

struct Ground;

struct Droplet
{
	gvec2 pos;
	gvec2 direction;
	gfloat speed;
	gvec2 grad;
	gfloat sediment;
	gfloat water;
	gfloat oldHeight;

	// debug
	gfloat newHeight;
	gfloat heightDiff;
	gfloat capacity;
	gfloat toDeposit;
	gfloat toErode;


	static constexpr gfloat pMomentum = 0.2;
	static constexpr gfloat pMinSlope = 0.01;
	static constexpr gfloat pCapacity = 4;
	static constexpr gfloat pDeposition = 0.3;
	static constexpr gfloat pErosion = 0.3;
	static constexpr gfloat pEvaporation = 0.01;
	static constexpr gfloat pGravity = 4;

	void init(Ground& g);

	bool step(Ground& g);


	void imguiRender();
};
