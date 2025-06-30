#pragma once
#include "ndpch.h"
#include "types.h"

namespace nd
{
	class NBT;
}

struct BaseGround;

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


	gfloat pMomentum = 0.2;
	gfloat pMinSlope = 0.01;
	gfloat pCapacity = 1.5;
	gfloat pDeposition = 0.3;
	gfloat pErosion = 0.3;
	gfloat pEvaporation = 0.01;
	gfloat pGravity = 4;
	int pRadius = 0;
	static constexpr int MAX_RADIUS = 16;

	// keep track of number of balls dropped
	static int balls;

	std::vector<gfloat> kernel;

	void init(BaseGround& g);

	bool step(BaseGround& g);


	void imguiRender();

	void save(nd::NBT& src);
	void load(nd::NBT& src);

	
};
