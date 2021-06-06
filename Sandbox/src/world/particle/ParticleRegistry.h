#pragma once
#include "ndpch.h"
#include "core/sids.h"

namespace nd {
class TextureAtlas;
}

typedef int ParticleID;
constexpr ParticleID INVALID_PARTICLE_ID = -1;
#define PARTICLE_ID(name)\
	ParticleRegistry::get().particle(name)

class ParticleRegistry
{
public:
	static ParticleRegistry& get() {
		static ParticleRegistry s_instance;
		return s_instance;
	}
	struct ParticleTemplate
	{
		std::string name;
		half_int texture_pos;
		half_int size;
		int frameCount;
		int ticksPerFrame;
		float rotationSpeed;

		ParticleTemplate(const std::string& name,half_int size,int frameCount,int ticksPerFrame, float rotationSpeed):
		name(name),size(size),frameCount(frameCount),ticksPerFrame(ticksPerFrame), rotationSpeed(rotationSpeed)
		{}

	};
private:
	std::vector<ParticleTemplate> m_templates;
	std::unordered_map<nd::Strid, ParticleID> m_ids;
	ParticleRegistry()=default;
public:
	ParticleRegistry(ParticleRegistry const&) = delete;
	void operator=(ParticleRegistry const&) = delete;

	ParticleID registerTemplate(const ParticleTemplate& t);

	void initTextures(const nd::TextureAtlas& atlas);

	const auto& getList() const { return m_templates; }

	ParticleID particle(const char* name)
	{
		auto& it = m_ids.find(SID(name));
		if (it == m_ids.end())
			return INVALID_PARTICLE_ID;
		return it->second;
	}
	const ParticleTemplate& getTemplate(const char* name)
	{
		ASSERT(particle(name) != INVALID_PARTICLE_ID, "Invalid template name");
		return getTemplate(particle(name));
	}
	const ParticleTemplate& getTemplate(ParticleID id) const
	{
		ASSERT(id >= 0 && id < m_templates.size(), "invalid ParticleID");
		return m_templates[id];
	}
};

