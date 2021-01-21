#include "ndpch.h"
#include "particles.h"
#include "core/NBT.h"
#define PARTICATOR(var) var= PARTICLE_ID(#var)

namespace ParticleList {

	void initDefaultParticles(ParticleRegistry& p)
	{
		NBT t;
		if (!NBT::loadFromFile(ND_RESLOC("res/registry/particles/particles.json"), t))
			return;
		std::string names;
		names.reserve(100);
		if (t.isMap())
		{
			for (auto& pair : t.maps())
			{
				auto& item = pair.second;
				std::string name=pair.first;
				names += ", ";
				names += name;
				
				int frameCount = 1;
				int ticksPerFrame = 10000;
				float rotationSpeed = 0;
				glm::vec2 s(1);
				item.load("size", s);
				
				item.load("frameCount",frameCount);
				item.load("ticksPerFrame", ticksPerFrame);
				item.load("rotationSpeed", rotationSpeed);
				ParticleRegistry::get().registerTemplate(ParticleRegistry::ParticleTemplate(name, half_int(s.x,s.y), frameCount, ticksPerFrame, rotationSpeed));
			}
		}
		ND_TRACE("Particles loaded: {}", names.substr(2));
		PARTICATOR(dot);
		PARTICATOR(line);
		PARTICATOR(torch_smoke);
		PARTICATOR(torch_fire);
		PARTICATOR(bulletShatter);
		PARTICATOR(block);
		PARTICATOR(note);
		/*dot = PARTICLE_ID("dot");
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
			ParticleRegistry::get().registerTemplate(ParticleRegistry::ParticleTemplate("note", half_int(1, 1), 4, 100000,0));*/
	}
}
