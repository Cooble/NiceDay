#pragma once
#include "core/physShapes.h"
#include "world/particle/ParticleRegistry.h"
#include "world/block/Block.h"

class ParticleRenderer;
class Texture;

constexpr int particleBlockDivision = 4;
constexpr uint8_t isBlockBit = (1 << 0);
constexpr uint8_t isLightedBit = (1 << 1);
class ParticleManager
{

private:
	
	struct Particle
	{
		glm::vec2 acceleration;
		glm::vec2 velocity;
		glm::vec2 pos;
		float rotation;
		float rotationSpeed;
		int life;
		ParticleID id;
		half_int textureIndex;
		half_int textureSize;
		uint8_t bits;
		uint8_t frameCount;
		uint8_t currentFrame;
		uint8_t nextFrame;
		uint8_t frameAge;
		// ticks per frame
		uint8_t  tpf;
		constexpr bool isBlock() { return bits & isBlockBit; }
		constexpr bool isLighted() { return bits & isLightedBit; }
	};
	Particle* m_list= nullptr;
	int m_list_size;
	int m_particleCount=0;
	Texture* m_atlas;
	Texture* m_block_atlas;
	int m_atlas_segment_count;
	int m_block_atlas_segment_count;
public:
	ParticleManager(int maxParticles,Texture* atlas,int atlasSegmentCount,Texture* blockAtlas,int blockAtlasSegmentCount);
	ParticleManager();
	~ParticleManager();

	void update();
	inline void clearAll() { m_particleCount = 0; }
	Particle& createParticle();


	// rotation if negative will be determined from actual speed of particle
	void createParticle(ParticleID id, const glm::vec2& pos, const glm::vec2& speed, const glm::vec2& acc, int life,float rotation=0);
	void createParticle(ParticleID id, const glm::vec2& pos, const glm::vec2& speed, const glm::vec2& acc, int life,float rotation,half_int texturePos);
	void createBlockParticle(half_int textureOffset, int xPos,int yPos, const glm::vec2& pos, const glm::vec2& speed, const glm::vec2& acc, int life,float rotation=0);
	void render(ParticleRenderer& renderer);
};
