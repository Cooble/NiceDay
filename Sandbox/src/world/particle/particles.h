#pragma once
#include "ParticleRegistry.h"

namespace ParticleList
{
	inline ParticleID torch_fire;
	inline ParticleID torch_smoke;
	inline ParticleID dot;
	inline ParticleID line;
	inline ParticleID bulletShatter;
	inline ParticleID block;

	void initDefaultParticles(ParticleRegistry& p);
};
