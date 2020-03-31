#pragma once

#include "world/entity/WorldEntity.h"
#include "world/entity/EntityRegistry.h"
#include "core/physShapes.h"
#include "PathTracer.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/IBatchRenderable2D.h"
#include "graphics/Sprite.h"
#include "graphics/Bar.h"
#include "inventory/Inventory.h"

class TileEntitySapling :public TileEntity
{
public:
	TileEntitySapling() = default;
	virtual ~TileEntitySapling() = default;
	void update(World& w) override;
	EntityType getEntityType() const override;
	

	TO_ENTITY_STRING(TileEntitySapling)
	ND_FACTORY_METH_ENTITY_BUILD(TileEntitySapling)
private:
	void buildTree(World& w, int x, int y);
};
class TileEntityTorch :public TileEntity
{
private:
	int m_tick_to_spawn_particle=1;
public:
	TileEntityTorch() = default;
	virtual ~TileEntityTorch() = default;
	void update(World& w) override;
	EntityType getEntityType() const override;

	TO_ENTITY_STRING(TileEntityTorch)
	ND_FACTORY_METH_ENTITY_BUILD(TileEntityTorch)
};
class TileEntityChest :public TileEntity
{
private:
	BasicInventory m_inventory;
	EntityID m_opener=ENTITY_ID_INVALID;
	bool m_shouldClose = false;
public:
	TileEntityChest();

	void onClicked(World& w, WorldEntity* entity) override;
	void update(World& w) override;
	inline Inventory& getInventory() { return m_inventory; }
	EntityType getEntityType() const override;
	
	void save(NBT& src) override;
	void load(NBT& src) override;

	TO_ENTITY_STRING(TileEntityChest)
	void onGUIEntityClosed();
	ND_FACTORY_METH_ENTITY_BUILD(TileEntityChest)
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
	glm::vec2 m_velocity;
	glm::vec2 m_max_velocity;
	glm::vec2 m_acceleration;
	Phys::Polygon m_bound;
	bool m_can_walk=true;


public:
	PhysEntity() = default;
	virtual ~PhysEntity() = default;

	//calculates velocity and position based on acceleration and world colliding blocks
	void computePhysics(World& w);
	bool moveOrCollide(World& w, float dt);
	//regards this as dimensionless structure, checks for collisions only blocks
	bool moveOrCollideOnlyBlocksNoBounds(World& w);
	inline void computeVelocity(World& w);
	inline void computeWindResistance(World& w,float windResistance=0.01f);

	void save(NBT& src) override;
	void load(NBT& src) override;

public:
	inline glm::vec2& getAcceleration() { return m_acceleration; }
	inline glm::vec2& getVelocity() { return m_velocity; }
	inline const Phys::Polygon& getCollisionBox() const { return m_bound; }
	inline bool isOnFloor() const { return m_is_on_floor; }
	inline Blockage getBlockageState() const { return m_blockage; }
};


class EntityItem:public PhysEntity, public IBatchRenderable2D
{
protected:
	int m_live_ticks;
	int m_max_live_ticks;
	int m_speed_mode_ticks_remaining=0;
protected:
	UVQuad m_sprite;
	float m_angle;
	ItemStack* m_item_stack;
	EntityID m_target=ENTITY_ID_INVALID;
	EntityID m_ignore_target=ENTITY_ID_INVALID;
	long long lastTime = 0;
	int m_ticks_to_new_search=10;
public:
	EntityItem();
	void setItemStack(ItemStack* stack);
	/***
	 * will ignore this entity for a short while after throwing it
	 * used to forbid the item to come back to the thrower
	 */
	inline void setThrowerEntity(EntityID id) { m_ignore_target = id; }
	void update(World & w) override;
	void render(BatchRenderer2D & renderer) override;
	EntityType getEntityType() const override;
	void save(NBT& src) override;
	void load(NBT& src) override;
	void onSpawned(World& w) override;

	TO_ENTITY_STRING(EntityItem)
	ND_FACTORY_METH_ENTITY_BUILD(EntityItem)
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
	EntityID m_owner_id=ENTITY_ID_INVALID;
	int m_ticks_to_ignore_owner=15;

public:
	Bullet();
	virtual ~Bullet() = default;
	void fire(float angle, float velocity);
	void fire(const glm::vec2& target, float velocity);
	void setOwner(EntityID getId);


	// return true if there was collision
	bool checkCollisions(World& w, float dt);
	void update(World& w) override;
	//return true if was hit
	virtual bool onBlockHit(World& w, int blockX, int blockY);
	//return true if was hit
	virtual bool onEntityHit(World& w, WorldEntity* entity);
	void render(BatchRenderer2D& renderer) override;
};

inline void Bullet::setOwner(EntityID id)
{
	m_owner_id = id;
	m_ticks_to_ignore_owner = 15;
}

class EntityRoundBullet : public Bullet,public LightSource
{
protected:
	float m_punch_back = 0.4f;
	
public:
	EntityRoundBullet();	
	virtual ~EntityRoundBullet() = default;

	EntityType getEntityType() const override;
	std::pair<int, int> getLightPosition() const override { return std::make_pair((int)m_pos.x, (int)m_pos.y); }
	uint8_t getIntensity() const override { return 10; }
	bool onEntityHit(World& w, WorldEntity* entity) override;
	bool onBlockHit(World& w, int blockX, int blockY) override;

	void onLoaded(World& w) override;
	void onUnloaded(World& w) override;

	TO_ENTITY_STRING(EntityRoundBullet)
	ND_FACTORY_METH_ENTITY_BUILD(EntityRoundBullet)

	/*void save(NBT2& src) override;
	void load(NBT2& src) override;*/
};

class Creature : public PhysEntity, public IBatchRenderable2D
{
protected:
	Animation m_animation;
	Bar m_health_bar;
	float m_max_health=1;
	float m_health=1;
	//always normalized
	glm::vec2 m_facing_direction;

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

	virtual void onHit(World& w,WorldEntity *e, float damage);

	virtual void throwItem(World& w,ItemStack* stack);
	inline void setFacingDirection(const glm::vec2& facing) { m_facing_direction = facing; }
	inline const glm::vec2& getFacingDirection() const { return m_facing_direction; }
	void save(NBT& src) override;
	void load(NBT& src) override;
};

inline void Creature::onHit(World& w,WorldEntity* e, float damage)
{
	m_health = max(0.f, m_health - damage);
	if (m_health == 0.f)
		markDead();
}

class EntityTNT : public Creature
{
private:
	int m_timeToBoom = 60 * 2;
	int m_blinkTime = 0;
	bool flip;

public:
	bool m_deleteWalls=false;
	int m_radius=5;
	
	EntityTNT();

	void update(World& w) override;
	void boom(World& w);
	EntityType getEntityType() const override;

	TO_ENTITY_STRING(EntityTNT)
	ND_FACTORY_METH_ENTITY_BUILD(EntityTNT)

	void save(NBT& src) override;
	void load(NBT& src) override;
};
class EntityBomb : public Creature
{
private:
	int m_timeToBoom = 60 * 2;
	int m_blinkTime = 0;
	bool flip;
	bool m_deleteWalls = false;
	int m_blastRadius = 5;

public:


	EntityBomb();

	void setBombType(int blastRadius, bool deleteWalls,int meta);
	void update(World& w) override;
	void boom(World& w);
	EntityType getEntityType() const override;

	TO_ENTITY_STRING(EntityBomb);
	ND_FACTORY_METH_ENTITY_BUILD(EntityBomb);

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

	void update(World& w) override;
	EntityType getEntityType() const override;

	TO_ENTITY_STRING(EntityZombie)
	ND_FACTORY_METH_ENTITY_BUILD(EntityZombie)

	void save(NBT& src) override;
	void load(NBT& src) override;
};

class EntitySnowman : public Creature
{
private:
	PathTracer m_tracer;
	bool m_found_player = false;
	float m_pose = 0;
	float m_last_pose = 0;
	int m_animation_var = 0;

public:
	EntitySnowman();

	void update(World& w) override;
	EntityType getEntityType() const override;

	TO_ENTITY_STRING(EntitySnowman)
	ND_FACTORY_METH_ENTITY_BUILD(EntitySnowman)

	void save(NBT& src) override;
	void load(NBT& src) override;
};
