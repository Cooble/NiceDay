#include "ndpch.h"
#include "ParticleManager.h"
#include "graphics/ParticleRenderer.h"
#include "graphics/API/Texture.h"
#include "core/Stats.h"
#include "graphics/Renderable2D.h"
#include "world/block/BlockRegistry.h"
#include "particles.h"
#include "graphics/Renderer.h"


ParticleManager::ParticleManager(int maxparticles, Texture* atlas, int atlasSegmentCount, Texture* blockAtlas,
                                 int blockAtlasSegmentCount):
	m_list_size(maxparticles),
	m_atlas(atlas),
	m_block_atlas(blockAtlas),
	m_atlas_segment_count(atlasSegmentCount),
	m_block_atlas_segment_count(blockAtlasSegmentCount)
{
	m_list = (Particle*)malloc(sizeof(Particle) * maxparticles);
}

ParticleManager::ParticleManager()
{
}

ParticleManager::~ParticleManager()
{
	if (m_list)
	{
		free(m_list);
		m_list = nullptr;
	}
}

void ParticleManager::update()
{
	Stats::particle_count = m_particleCount;
	for (int i = 0; i < m_particleCount; ++i)
	{
		auto& particle = m_list[i];
		if (particle.life <= 0)
		{
			memcpy(&particle, &m_list[m_particleCount - 1], sizeof(Particle));
			--m_particleCount;
		}
		particle.velocity += particle.acceleration;
		particle.pos += particle.velocity;
		if (particle.rotation < 0)
		{
			particle.rotation = -2 * 3.1415926535f + Phys::angleRad(particle.velocity);
		}
		else
			particle.rotation += particle.rotationSpeed;
		++particle.frameAge;
		if (particle.frameAge >= particle.tpf)
		{
			particle.frameAge = 0;
			particle.currentFrame = particle.nextFrame;
			particle.nextFrame = (particle.currentFrame + 1) % particle.frameCount;
		}
		--particle.life;
	}
}

ParticleManager::Particle& ParticleManager::createParticle()
{
	//ASSERT(m_particleCount + 1 < m_list_size, "Too small particle buffer");
	if (!(m_particleCount + 1 < m_list_size))
		//sacrifice the old half of particles to make room for more(not perfect but still better than debugkbreak or spooky silent buffer overflow)
	{
		memcpy(m_list, m_list + m_list_size / 2 - 2, m_list_size / 2);
		m_particleCount = m_list_size / 2;
	}
	return m_list[m_particleCount++];
}


void ParticleManager::createParticle(ParticleID id, const glm::vec2& pos, const glm::vec2& speed, const glm::vec2& acc,
                                     int life, float rotation)
{
	auto& p = createParticle();
	auto& templ = ParticleRegistry::get().getTemplate(id);
	p.id = id;
	p.pos = pos;
	p.velocity = speed;
	p.acceleration = acc;
	p.life = life==0?templ.ticksPerFrame*templ.frameCount:life;
	p.textureIndex = templ.texture_pos;
	p.currentFrame = 0;
	p.nextFrame = templ.frameCount == 1 ? 0 : 1;
	p.frameCount = templ.frameCount;
	p.textureSize = templ.size;
	p.tpf = templ.ticksPerFrame;
	p.rotation = rotation;
	p.rotationSpeed = templ.rotationSpeed;
	p.frameAge = 0;
	p.bits = isLightedBit;
}

void ParticleManager::createParticle(ParticleID id, const glm::vec2& pos, const glm::vec2& speed, const glm::vec2& acc,
                                     int life, float rotation, half_int texturePos)
{
	auto& p = createParticle();
	auto& templ = ParticleRegistry::get().getTemplate(id);
	p.id = id;
	p.pos = pos;
	p.velocity = speed;
	p.acceleration = acc;
	p.life = life == 0 ? templ.ticksPerFrame * templ.frameCount : life;
	p.textureIndex = templ.texture_pos + texturePos;
	p.currentFrame = 0;
	p.nextFrame = templ.frameCount == 1 ? 0 : 1;
	p.frameCount = templ.frameCount;
	p.textureSize = templ.size;
	p.tpf = templ.ticksPerFrame;
	p.rotation = rotation;
	p.rotationSpeed = templ.rotationSpeed;
	p.frameAge = 0;
	p.bits = isLightedBit;
}


void ParticleManager::createBlockParticle(half_int textureOffset, int xPos, int yPos, const glm::vec2& pos,
                                          const glm::vec2& speed, const glm::vec2& acc, int life, float rotation)
{
	auto& p = createParticle();
	auto& templ = ParticleRegistry::get().getTemplate(ParticleList::block);

	p.id = ParticleList::block;
	p.pos = pos;
	p.velocity = speed;
	p.acceleration = acc;
	p.life = life;
	p.textureIndex = (textureOffset * particleBlockDivision) + half_int(xPos, yPos);
	p.currentFrame = 0;
	p.nextFrame = 1;
	p.frameCount = 2;
	p.textureSize = half_int(1, 1);
	p.tpf = templ.ticksPerFrame;
	p.rotation = rotation;
	p.rotationSpeed = templ.rotationSpeed;
	p.frameAge = 0;
	p.bits = isBlockBit;
}

void ParticleManager::render(ParticleRenderer& renderer)
{
	ND_PROFILE_METHOD();

	float piece1 = 1.f / m_atlas_segment_count;
	float piece2 = 1.f / m_block_atlas_segment_count / particleBlockDivision;
	float blockPiece = 1.f / particleBlockDivision;
	glm::vec2 blockPieceVec(blockPiece);


	for (int i = 0; i < m_particleCount; ++i)
	{
		auto& particle = m_list[i];
		if(particle.life<=0)
			continue;
		UVQuad quad0;

		auto piece = particle.isBlock() ? piece2 : piece1;
		glm::vec2 siz = glm::vec2(particle.textureSize.x, particle.textureSize.y) * piece;
		quad0.uv[0] = glm::vec2((particle.textureIndex.x + (int)particle.currentFrame * particle.textureSize.x),
		                        particle.textureIndex.y) * piece;
		quad0.uv[1] = quad0.uv[0];
		quad0.uv[3] = quad0.uv[0];
		quad0.uv[2] = quad0.uv[0] + siz;
		quad0.uv[1].x += siz.x;
		quad0.uv[3].y += siz.y;

		//quad0.setPosition(glm::vec2((particle.textureIndex.x + (int)particle.currentFrame * particle.textureSize.x), particle.textureIndex.y) * piece);
		//quad0.setSize(glm::vec2(particle.textureSize.x, particle.textureSize.y) * piece);


		//quad1.setPosition(glm::vec2(particle.textureIndex.x + (int)particle.nextFrame * particle.textureSize.x, particle.textureIndex.y) * piece);
		//quad1.setSize(glm::vec2(particle.textureSize.x, particle.textureSize.y) * piece);

		auto m = glm::translate(glm::mat4(1.f), glm::vec3(particle.pos.x, particle.pos.y, 0.1f));
		m = glm::rotate(m, particle.rotation, glm::vec3(0.f, 0.f, 1.f));
		renderer.push(m);
		if (!particle.isBlock())
		{
			UVQuad quad1;
			quad1.uv[0] = glm::vec2(particle.textureIndex.x + (int)particle.nextFrame * particle.textureSize.x,
				particle.textureIndex.y) * piece;
			quad1.uv[1] = quad1.uv[0];
			quad1.uv[3] = quad1.uv[0];
			quad1.uv[2] = quad1.uv[0] + siz;
			quad1.uv[1].x += siz.x;
			quad1.uv[3].y += siz.y;
			float mix = (float)particle.frameAge / particle.tpf * (particle.life<particle.tpf?-1.f:1.f);
			//float mix = (float)particle.frameAge / particle.tpf ;
			
			renderer.submit(glm::vec3(-particle.textureSize.x / 2.f, -particle.textureSize.y / 2.f,
			                          -0.5f + (float)particle.isLighted()),
			                glm::vec2(particle.textureSize.x, particle.textureSize.y),
			                quad0, quad1,
			                m_atlas,
			                mix);
		}
		else
		{
			UVQuad quad1;

			quad1.uv[0] = glm::vec2(0);
			quad1.uv[1] = glm::vec2(siz.x, 0.f);
			quad1.uv[3] = glm::vec2(0.f, siz.y);
			quad1.uv[2] = siz;

			//quad1.setPosition(glm::vec2(0) * piece);
			//quad1.setSize(glm::vec2(particle.textureSize.x, particle.textureSize.y) * piece);

			renderer.submit(glm::vec3(-blockPiece / 2, -blockPiece / 2, -0.5f + (float)particle.isLighted()),
			                blockPieceVec,
			                quad0, quad1,
			                m_block_atlas,
			                ((float)particle.frameAge) / particle.tpf);
		}
		renderer.pop();
	}
}
