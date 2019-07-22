#pragma once

#include "world/entity/WorldEntity.h"
#include "world/entity/EntityRegistry.h"
#include "physShapes.h"
#include "PathTracer.h"

class PhysEntity :public WorldEntity
{
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

	void save(NBT& src) override;
	void load(NBT& src) override;

public:
	inline Phys::Vect& getAcceleration() { return m_acceleration; }
	inline Phys::Vect& getVelocity() { return m_velocity; }
	inline const Phys::Polygon& getBoundPolygon()const { return m_bound; }

};
class Creature :public PhysEntity,public IBatchRenderable2D
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
class EntityPlayer :public Creature
{
private:
	std::string m_name = "Karel";
	int m_pose=0;

public:
	EntityPlayer();

	void update(World* w) override;
	EntityType getEntityType() const override;

	TO_ENTITY_STRING(EntityPlayer)
	ND_FACTORY_METH_ENTITY_BUILD(EntityPlayer)

	void save(NBT& src) override;
	void load(NBT& src) override;

};
class EntityTNT :public Creature
{
private:
	int m_timeToBoom =60*2;
	int m_blinkTime=0;
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
class EntityZombie :public Creature
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