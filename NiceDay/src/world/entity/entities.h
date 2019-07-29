#pragma once

#include "world/entity/WorldEntity.h"
#include "world/entity/EntityRegistry.h"
#include "physShapes.h"
#include "PathTracer.h"

class PhysEntity : public WorldEntity
{
public:
	enum Blockage
	{
		LEFT,
		RIGHT,
		STUCK,
		NONE,
	};

private:
	bool m_is_on_floor;
	Blockage m_blockage;
protected:
	Phys::Vect m_velocity;
	Phys::Vect m_max_velocity;
	Phys::Vect m_acceleration;
	Phys::Polygon m_bound;


public:
	PhysEntity() = default;
	virtual ~PhysEntity() = default;

	//calculates velocity and position based on acceleration and world colliding blocks
	void computePhysics(World* w);
	inline void moveOrCollide(World* w);
	inline void computeVelocity(World* w);
	inline void computeWindResistance(World* w);

	void save(NBT& src) override;
	void load(NBT& src) override;

public:
	inline Phys::Vect& getAcceleration() { return m_acceleration; }
	inline Phys::Vect& getVelocity() { return m_velocity; }
	inline const Phys::Polygon& getBoundPolygon() const { return m_bound; }
	inline bool isOnFloor() const { return m_is_on_floor; }
	inline Blockage getBlockageState() const { return m_blockage; }
};

constexpr float MAX_BULLET_ENTITY_DISTANCE_SQ = 25 * 25;

class Bullet : public PhysEntity, public IBatchRenderable2D
{
protected:
	/*struct BulletTemplate
	{*/
	float m_damage;
	int m_live_ticks=0;
	int m_max_live_ticks=std::numeric_limits<int>::max();
	/*/};*/
protected:
	//BulletTemplate* m_template;
	Sprite m_sprite;
	float m_angle;

public:
	Bullet() = default;
	virtual ~Bullet() = default;
	void fire(float angle, float velocity);
	void fire(const Phys::Vect target, float velocity);


	void update(World* w) override;
	virtual void onBlockHit(World* w, int blockX, int blockY);
	virtual void onCreatureHit(World* w, WorldEntity* entity);
	void render(BatchRenderer2D& renderer) override;
};

class EntityRoundBullet : public Bullet,public LightSource
{
public:
	EntityRoundBullet();

	EntityType getEntityType() const override;
	std::pair<int, int> getLightPosition() const override { return std::make_pair(m_pos.x, m_pos.y); }
	uint8_t getIntensity() const override { return 10; }

	void onLoaded(World* w) override;
	void onUnloaded(World* w) override;
	TO_ENTITY_STRING(RoundBullet)
	ND_FACTORY_METH_ENTITY_BUILD(EntityRoundBullet)

	/*void save(NBT& src) override;
	void load(NBT& src) override;*/
};


class Creature : public PhysEntity, public IBatchRenderable2D
{
protected:

	Sprite m_sprite;

public:
	Creature() = default;
	virtual ~Creature() = default;

	inline Sprite& getSprite() { return m_sprite; }
	void render(BatchRenderer2D& renderer) override;

	void save(NBT& src) override;
	void load(NBT& src) override;
};

class EntityPlayer : public Creature
{
private:
	std::string m_name = "Karel";
	int m_pose = 0;

public:
	EntityPlayer();

	void update(World* w) override;
	EntityType getEntityType() const override;

	TO_ENTITY_STRING(EntityPlayer)
	ND_FACTORY_METH_ENTITY_BUILD(EntityPlayer)

	void save(NBT& src) override;
	void load(NBT& src) override;
};

class EntityTNT : public Creature
{
private:
	int m_timeToBoom = 60 * 2;
	int m_blinkTime = 0;
	bool flip;

public:
	EntityTNT();

	void update(World* w) override;
	EntityType getEntityType() const override;

	TO_ENTITY_STRING(EntityTNT)
	ND_FACTORY_METH_ENTITY_BUILD(EntityTNT)

	void save(NBT& src) override;
	void load(NBT& src) override;
};

class EntityZombie : public Creature
{
private:
	int m_pose = 0;
	PathTracer m_tracer;
	bool m_found_player = false;

public:
	EntityZombie();

	void update(World* w) override;
	EntityType getEntityType() const override;

	TO_ENTITY_STRING(EntityZombie)
	ND_FACTORY_METH_ENTITY_BUILD(EntityZombie)

	void save(NBT& src) override;
	void load(NBT& src) override;
};
