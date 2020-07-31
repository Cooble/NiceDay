#include "ndpch.h"
#include "ParticleRegistry.h"
#include "graphics/TextureAtlas.h"


ParticleID ParticleRegistry::registerTemplate(const ParticleTemplate& t)
{
	m_templates.push_back(t);
	m_ids[SID(t.name)] = m_templates.size() - 1;
	return m_templates.size() - 1;
}

void ParticleRegistry::initTextures(const TextureAtlas& atlas)
{
	for (auto& t : m_templates)
		t.texture_pos = atlas.getTexture(t.name);
}
