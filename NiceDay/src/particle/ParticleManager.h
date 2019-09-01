#pragma once
#include "physShapes.h"
#include "world/particle/ParticleRegistry.h"

class ParticleRenderer;
class Texture;

class ParticleManager
{
private:
	struct Particle
	{
		Phys::Vect acceleration;
		Phys::Vect velocity;
		Phys::Vect pos;
		float rotation;
		float rotationSpeed;
		int life;
		ParticleID id;
		half_int texture_index;
		half_int texture_size;
		uint8_t frame_count;
		uint8_t current_frame;
		uint8_t next_frame;
		uint8_t frame_age;
		// ticks per frame
		uint8_t  tpf;

	};
	Particle* m_list= nullptr;
	int m_list_size;
	int m_particleCount=0;
	Texture* m_atlas;
	int m_atlas_segment_count;
public:
	ParticleManager(int maxParticles,Texture* atlas,int atlasSegmentCount);
	ParticleManager();
	~ParticleManager();

	void update();
	inline void clearAll() { m_particleCount = 0; }
	Particle& createParticle();


	// rotation if negative will be determined from actual speed of particle
	void createParticle(ParticleID id, Phys::Vect pos, Phys::Vect speed, Phys::Vect acc, int life,float rotation=0);
	void render(ParticleRenderer& renderer);
};
