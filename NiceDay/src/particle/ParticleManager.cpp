#include "ndpch.h"
#include "ParticleManager.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/Renderable2D.h"
#include "graphics/Texture.h"
#include "Stats.h"


ParticleManager::ParticleManager(int maxparticles,Texture* atlas,int atlasSegmentCount): m_list_size(maxparticles),m_atlas(atlas),m_atlas_segment_count(atlasSegmentCount){
	m_list = (Particle*) malloc(sizeof(Particle)*maxparticles);
}

ParticleManager::ParticleManager()
:m_list(nullptr){
}

ParticleManager::~ParticleManager()
{
	if (m_list) {
		free(m_list);
		m_list = nullptr;
	}
}

void ParticleManager::update()
{
	//TimerStaper t("update of particles took ");
	Stats::particle_count = m_particleCount;
	for (int i = 0; i < m_particleCount; ++i)
	{
		auto& particle = m_list[i];
		if(particle.life<=0)
		{
			memcpy(&particle, &m_list[m_particleCount - 1], sizeof(Particle));
			--m_particleCount;
		}
		particle.velocity += particle.acceleration;
		particle.pos += particle.velocity;
		particle.rotation += particle.rotationSpeed;
		++particle.frame_age;
		if(particle.frame_age>=particle.tpf)
		{
			particle.frame_age = 0;
			particle.current_frame = (particle.current_frame+1)%particle.frame_count;
		}
		--particle.life;
	}
}

ParticleManager::Particle& ParticleManager::createParticle()
{
	return m_list[m_particleCount++];
}

void ParticleManager::createParticle(ParticleID id, Phys::Vect pos, Phys::Vect speed, Phys::Vect acc, int life)
{

	Particle& p = createParticle();
	auto& templ = ParticleRegistry::get().getTemplate(id);
	p.id = id;
	p.pos = pos-Phys::Vect(templ.size.x/2.f, templ.size.y / 2.f);
	p.velocity = speed;
	p.acceleration = acc;
	p.life = life;
	p.texture_index = templ.texture_pos;
	p.current_frame = 0;
	p.frame_count = templ.frameCount;
	p.texture_size = templ.size;
	p.tpf = templ.ticksPerFrame;
	p.rotationSpeed = templ.rotationSpeed;

}

void ParticleManager::render(BatchRenderer2D& renderer)
{

	//TimerStaper t("render of particles took ");

	float piece = 1.f/m_atlas_segment_count;

	for (int i = 0; i < m_particleCount; ++i)
	{
		auto& particle = m_list[i];
		UVQuad quad;

		quad.setPosition(glm::vec2((particle.texture_index.x+particle.current_frame)*piece, particle.texture_index.y*piece));
		quad.setSize(glm::vec2(particle.texture_size.x*piece, particle.texture_size.y*piece));
		//renderer.push(glm::rotate(glm::mat4(1.f), particle.rotation, glm::vec3(0.f, 0.f,1.f)));
		renderer.submit(glm::vec3(particle.pos.x, particle.pos.y, 0.1f), glm::vec2(particle.texture_size.x, particle.texture_size.y), quad, m_atlas);
		//renderer.pop();
	}
}
