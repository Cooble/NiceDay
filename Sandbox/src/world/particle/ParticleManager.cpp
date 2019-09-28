#include "ndpch.h"
#include "ParticleManager.h"
#include "graphics/ParticleRenderer.h"
#include "graphics/API/Texture.h"
#include "Stats.h"
#include "graphics/Renderable2D.h"


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
		if(particle.rotation<0)
		{
			particle.rotation = Phys::toRad(-360+particle.velocity.angleDegrees());
		}else
			particle.rotation += particle.rotationSpeed;
		++particle.frame_age;
		if(particle.frame_age>=particle.tpf)
		{
			particle.frame_age = 0;
			particle.current_frame = particle.next_frame;
			particle.next_frame = (particle.current_frame+1)%particle.frame_count;
		}
		--particle.life;
	}
}

ParticleManager::Particle& ParticleManager::createParticle()
{
	return m_list[m_particleCount++];
}


void ParticleManager::createParticle(ParticleID id, Phys::Vect pos, Phys::Vect speed, Phys::Vect acc, int life,float rotation)
{

	Particle& p = createParticle();
	auto& templ = ParticleRegistry::get().getTemplate(id);
	p.id = id;
	p.pos = pos;
	p.velocity = speed;
	p.acceleration = acc;
	p.life = life;
	p.texture_index = templ.texture_pos;
	p.current_frame = 0;
	p.next_frame = templ.frameCount==1?0:1;
	p.frame_count = templ.frameCount;
	p.texture_size = templ.size;
	p.tpf = templ.ticksPerFrame;
	p.rotation = rotation;
	p.rotationSpeed = templ.rotationSpeed;
	p.frame_age = 0;

}

void ParticleManager::render(ParticleRenderer& renderer)
{
	//TimerStaper t("render of particles took ");

	float piece = 1.f / m_atlas_segment_count;

	for (int i = 0; i < m_particleCount; ++i)
	{
		auto& particle = m_list[i];
		UVQuad quad0;

		quad0.setPosition(glm::vec2((particle.texture_index.x + (int)particle.current_frame*particle.texture_size.x)*piece, particle.texture_index.y*piece));
		quad0.setSize(glm::vec2(particle.texture_size.x*piece, particle.texture_size.y*piece));

		UVQuad quad1;

		quad1.setPosition(glm::vec2((particle.texture_index.x + (int)particle.next_frame*particle.texture_size.x)*piece, particle.texture_index.y*piece));
		quad1.setSize(glm::vec2(particle.texture_size.x*piece, particle.texture_size.y*piece));

		auto m = glm::translate(glm::mat4(1.f), glm::vec3(particle.pos.x, particle.pos.y, 0.1f));
		m = glm::rotate(m, particle.rotation, glm::vec3(0.f, 0.f, 1.f));
		renderer.push(m);
		renderer.submit(glm::vec3(-particle.texture_size.x / 2.f, -particle.texture_size.y / 2.f, 0.0f),
			glm::vec2(particle.texture_size.x, particle.texture_size.y), 
			quad0,quad1,
			m_atlas,((float)particle.frame_age)/particle.tpf);
		renderer.pop();
	}
}
