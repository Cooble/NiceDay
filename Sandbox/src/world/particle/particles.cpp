#include "ndpch.h"
#include "particles.h"

namespace ParticleList {

	void initDefaultParticles(ParticleRegistry& p)
	{
		dot = 
			ParticleRegistry::get().registerTemplate(ParticleRegistry::ParticleTemplate("dot", half_int(1, 1), 4, 20, 0));
		line = 
			ParticleRegistry::get().registerTemplate(ParticleRegistry::ParticleTemplate("line", half_int(2, 1), 1, 0, 0));
		torch_smoke = 
			ParticleRegistry::get().registerTemplate(ParticleRegistry::ParticleTemplate("torch_smoke", half_int(1, 1), 5, 60,0));
		torch_fire = 
			ParticleRegistry::get().registerTemplate(ParticleRegistry::ParticleTemplate("torch_fire", half_int(1, 1), 4, 30, 0));
		bulletShatter = 
			ParticleRegistry::get().registerTemplate(ParticleRegistry::ParticleTemplate("bulletShatter", half_int(1,1), 2, 60, 0));
		block = 
			ParticleRegistry::get().registerTemplate(ParticleRegistry::ParticleTemplate("block", half_int(1, 1), 2, 35,0));
		note = 
			ParticleRegistry::get().registerTemplate(ParticleRegistry::ParticleTemplate("note", half_int(1, 1), 4, 100000,0));
	}
}
