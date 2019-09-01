#pragma once

#include "world/entity/WorldEntity.h"
#include "world/entity/EntityRegistry.h"
#include "physShapes.h"
#include "PathTracer.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/IBatchRenderable2D.h"
#include "graphics/Sprite.h"
#include "graphics/Bar.h"

class TileEntitySapling :public TileEntity
{
public:
	TileEntitySapling() = default;
	virtual ~TileEntitySapling() = default;
	void update(World* w) override;
	EntityType getEntityType() const override;

	
	

	TO_ENTITY_STRING(TileEntitySapling)
	ND_FACTORY_METH_ENTITY_BUILD(TileEntitySapling)
private:
	void buildTree(World* w, int x, int y);
};
class TileEntityTorch :public TileEntity
{
private:
	int m_tick_to_spawn_particle=1;
public:
	TileEntityTorch() = default;
	virtual ~TileEntityTorch() = default;
	void update(World* w) override;
	EntityType getEntityType() const override;

	TO_ENTITY_STRING(TileEntityTorch)
	ND_FACTORY_METH_ENTITY_BUILD(TileEntityTorch)
};

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
	bool m_can_walk=true;


public:
	PhysEntity() = default;
	virtual ~PhysEntity() = default;

	//calculates velocity and position based on acceleration and world colliding blocks
	void computePhysics(World* w);
	inline bool moveOrCollide(World* w, float dt);
	inline void computeVelocity(World* w);
	inline void computeWindResistance(World* w);

	void save(NBT& src) override;
	void load(NBT& src) override;

public:
	inline Phys::Vect& getAcceleration() { return m_acceleration; }
	inline Phys::Vect& getVelocity() { return m_velocity; }
	inline const Phys::Polygon& getCollisionBox() const { return m_bound; }
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
	int m_live_ticks;
	int m_max_live_ticks;
	/*/};*/
protected:
	//BulletTemplate* m_template;
	Sprite m_sprite;
	float m_angle;

public:
	Bullet();
	virtual ~Bullet() = default;
	void fire(float angle, float velocity);
	void fire(const Phys::Vect target, float velocity);


	// return true if there was collision
	bool checkCollisions(World* w, float dt);
	void update(World* w) override;
	//return true if was hit
	virtual bool onBlockHit(World* w, int blockX, int blockY);
	//return true if was hit
	virtual bool onEntityHit(World* w, WorldEntity* entity);
	void render(BatchRenderer2D& renderer) override;
};

class EntityRoundBullet : public Bullet,public LightSource
{
protected:
	float m_punch_back = 0.4f;
	
public:
	EntityRoundBullet();	
	virtual ~EntityRoundBullet() = default;

	EntityType getEntityType() const override;
	std::pair<int, int> getLightPosition() const override { return std::make_pair(m_pos.x, m_pos.y); }
	uint8_t getIntensity() const override { return 10; }
	bool onEntityHit(World* w, WorldEntity* entity) override;
	bool onBlockHit(World* w, int blockX, int blockY) override;

	void onLoaded(World* w) override;
	void onUnloaded(World* w) override;

	TO_ENTITY_STRING(EntityRoundBullet)
	ND_FACTORY_METH_ENTITY_BUILD(EntityRoundBullet)

	/*void save(NBT& src) override;
	void load(NBT& src) override;*/
};

class Creature : public PhysEntity, public IBatchRenderable2D
{
protected:
	Animation m_animation;
	Bar m_health_bar;
	float m_max_health=1;
	float m_health=1;

	inline void setMaxHealth(float maxHealth)
	{
		m_max_health = maxHealth;
		m_health = maxHealth;
	}

public:
	Creature();
	virtual ~Creature() = default;

	inline Sprite& getSprite() { return m_animation; }
	void render(BatchRenderer2D& renderer) override;

	virtual void onHit(World* w,WorldEntity *e, float damage);

	void save(NBT& src) override;
	void load(NBT& src) override;
};

inline void Creature::onHit(World* w,WorldEntity* e, float damage)
{
	m_health = max(0.f, m_health - damage);
	if (m_health == 0.f)
		markDead();
}

class EntityPlayer : public Creature
{
private:
	std::string m_name = "Karel";
	float m_pose = 0;
	float m_last_pose = 0;
	int m_animation_var = 0;


public:
	EntityPlayer();
	virtual ~EntityPlayer() = default;

	void update(World* w) override;
	EntityType getEntityType() const override;

	void onHit(World* w, WorldEntity* e, float damage) override;

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
	PathTracer m_tracer;
	bool m_found_player = false;
	float m_pose = 0;
	float m_last_pose = 0;
	int m_animation_var = 0;

public:
	EntityZombie();

	void update(World* w) override;
	EntityType getEntityType() const override;

	TO_ENTITY_STRING(EntityZombie)
	ND_FACTORY_METH_ENTITY_BUILD(EntityZombie)

	void save(NBT& src) override;
	void load(NBT& src) override;
};
