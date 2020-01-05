#include "ndpch.h"
#include "ParticleManager.h"
#include "graphics/ParticleRenderer.h"
#include "graphics/API/Texture.h"
#include "core/Stats.h"
#include "graphics/Renderable2D.h"
#include "world/block/BlockRegistry.h"
#include "particles.h"


ParticleManager::ParticleManager(int maxparticles,Texture* atlas,int atlasSegmentCount, Texture* blockAtlas, int blockAtlasSegmentCount):
m_list_size(maxparticles),m_atlas(atlas),m_block_atlas(blockAtlas),m_atlas_segment_count(atlasSegmentCount), m_block_atlas_segment_count(blockAtlasSegmentCount){
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
			particle.rotation = Phys::toRad(-360+Phys::Vect(particle.velocity).angleDegrees());
		}else
			particle.rotation += particle.rotationSpeed;
		++particle.frameAge;
		if(particle.frameAge>=particle.tpf)
		{
			particle.frameAge = 0;
			particle.currentFrame = particle.nextFrame;
			particle.nextFrame = (particle.currentFrame+1)%particle.frameCount;
		}
		--particle.life;
	}
}

ParticleManager::Particle& ParticleManager::createParticle()
{
	//ASSERT(m_particleCount + 1 < m_list_size, "Too small particle buffer");
	if(!(m_particleCount + 1 < m_list_size))//sacrifice the old half of particles to make room for more(not perfect but still better than debugkbreak or spooky silent buffer overflow)
	{
		memcpy(m_list, m_list + m_list_size / 2-2, m_list_size / 2);
		m_particleCount = m_list_size / 2;
	}
	return m_list[m_particleCount++];
}


void ParticleManager::createParticle(ParticleID id, const glm::vec2& pos, const glm::vec2& speed, const glm::vec2& acc, int life,float rotation)
{

	auto& p = createParticle();
	auto& templ = ParticleRegistry::get().getTemplate(id);
	p.isblock = false;
	p.id = id;
	p.pos = pos;
	p.velocity = speed;
	p.acceleration = acc;
	p.life = life;
	p.textureIndex = templ.texture_pos;
	p.currentFrame = 0;
	p.nextFrame = templ.frameCount==1?0:1;
	p.frameCount = templ.frameCount;
	p.textureSize = templ.size;
	p.tpf = templ.ticksPerFrame;
	p.rotation = rotation;
	p.rotationSpeed = templ.rotationSpeed;
	p.frameAge = 0;

}


void ParticleManager::createBlockParticle(half_int textureOffset, int xPos, int yPos, const glm::vec2& pos, const glm::vec2& speed, const glm::vec2& acc, int life, float rotation)
{
	auto& p = createParticle();
	auto& templ = ParticleRegistry::get().getTemplate(ParticleList::block);
	p.isblock = true;
	p.id = ParticleList::block;
	p.pos = pos;
	p.velocity = speed;
	p.acceleration = acc;
	p.life = life;
	p.textureIndex = (textureOffset * particleBlockDivision) + half_int(xPos,yPos);
	p.currentFrame = 0;
	p.nextFrame = 1;
	p.frameCount = 2;
	p.textureSize = half_int(1, 1);
	p.tpf = templ.ticksPerFrame;
	p.rotation = rotation;
	p.rotationSpeed = templ.rotationSpeed;
	p.frameAge = 0;

}

void ParticleManager::render(ParticleRenderer& renderer)
{
	//TimerStaper t("rp");//todo this method is incredibly slow

	float piece1 = 1.f / m_atlas_segment_count;
	float piece2 = 1.f / m_block_atlas_segment_count / particleBlockDivision;
	float blockPiece = 1.f / particleBlockDivision;

	for (int i = 0; i < m_particleCount; ++i)
	{
		auto& particle = m_list[i];
		UVQuad quad0;

		auto piece = particle.isblock ? piece2 : piece1;
		quad0.setPosition(glm::vec2((particle.textureIndex.x + (int)particle.currentFrame*particle.textureSize.x), particle.textureIndex.y)*piece);
		quad0.setSize(glm::vec2(particle.textureSize.x, particle.textureSize.y)*piece);

		UVQuad quad1;

		quad1.setPosition(glm::vec2(particle.textureIndex.x + (int)particle.nextFrame*particle.textureSize.x, particle.textureIndex.y)*piece);
		quad1.setSize(glm::vec2(particle.textureSize.x, particle.textureSize.y)*piece);

		auto m = glm::translate(glm::mat4(1.f), glm::vec3(particle.pos.x, particle.pos.y, 0.1f));
		m = glm::rotate(m, particle.rotation, glm::vec3(0.f, 0.f, 1.f));
		renderer.push(m);
		if (!particle.isblock) {
			renderer.submit(glm::vec3(-particle.textureSize.x / 2.f, -particle.textureSize.y / 2.f, 0.0f),
				glm::vec2(particle.textureSize.x, particle.textureSize.y),
				quad0, quad1,
				particle.isblock ? m_block_atlas : m_atlas, ((float)particle.frameAge) / particle.tpf);

		}else
		{
			UVQuad quad1;

			quad1.setPosition(glm::vec2(0) * piece);
			quad1.setSize(glm::vec2(particle.textureSize.x, particle.textureSize.y) * piece);
			
			renderer.submit(glm::vec3(-blockPiece/2, -blockPiece/2, 0.0f),
				glm::vec2(blockPiece, blockPiece),
				quad0, quad1,
				particle.isblock ? m_block_atlas : m_atlas, ((float)particle.frameAge) / particle.tpf);
		}
		renderer.pop();
	}
}
