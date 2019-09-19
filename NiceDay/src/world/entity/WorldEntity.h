#pragma once
#include "ndpch.h"
#include "EntityManager.h"
#include "world/WorldTime.h"



constexpr int EFLAG_TEMPORARY =		BIT(0);//will be killed on chunk unload
constexpr int EFLAG_CHUNK_LOADER =	BIT(1);//will keep chunks around loaded (usually Player)

class World;
struct WorldTime;

class WorldEntity: public NBTSaveable
{
	friend World;
private:
	bool m_is_dead=false;
protected:
	EntityID m_id;// each entity instance has unique id
protected:
	uint64_t m_flags=0;
	Phys::Vect m_pos;//world pos
protected:
	WorldEntity() = default;
	virtual ~WorldEntity() = default;
public:
	inline bool hasFlag(uint64_t flags) const { return (m_flags & flags)==flags; }

	inline bool isMarkedDead()const { return m_is_dead; }

	inline EntityID getID() const { return m_id; }
	
	inline void markDead() { m_is_dead = true; }//used for suicide (no one else can call this method)

	virtual EntityType getEntityType() const = 0;

	inline const Phys::Vect& getPosition() const { return m_pos; }
	inline Phys::Vect& getPosition() { return m_pos; }

	virtual void update(World& w) {}

	virtual void onLoaded(World& w) {}
	virtual void onUnloaded(World& w) {}

	virtual void onSpawned(World& w){}
	virtual void onKilled(World& w){}


	void save(NBT& src) override;
	void load(NBT& src) override;

#ifdef ND_DEBUG
	inline virtual std::string toString() const { return "UNDEFINED_ENTITY"; }

#define TO_ENTITY_STRING(x)\
	inline std::string toString() const override {return #x;}
#else
#define TO_ENTITY_STRING(X)
#endif

};

class TileEntity :public WorldEntity
{
protected:
	WorldTime m_last_update_ticks;
	WorldTime m_age;
public:
	TileEntity();
	virtual ~TileEntity() = default;

	void update(World& w) override;

	void onSpawned(World& w) override;

	void save(NBT& src) override;
	void load(NBT& src) override;

	inline int getX()const { return m_pos.x; }
	inline int getY()const { return m_pos.y; }
};


