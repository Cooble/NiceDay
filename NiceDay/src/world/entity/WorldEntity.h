#pragma once
#include "ndpch.h"
#include "EntityManager.h"
#include "graphics/IBatchRenderable2D.h"
#include "graphics/Sprite.h"



constexpr int EFLAG_TEMPORARY =		BIT(0);//will be killed on chunk unload
constexpr int EFLAG_CHUNK_LOADER =	BIT(1);//will keep chunks around loaded (usually Player)

class World;

class WorldEntity: public NBTSaveable
{
	friend World;
protected:
	EntityID m_id;// each entity instance has unique id
protected:
	uint64_t m_flags;
	Phys::Vect m_pos;//world pos
protected:
	WorldEntity() = default;
	virtual ~WorldEntity() = default;

public:
	inline bool hasFlag(uint64_t flags) const { return (m_flags & flags)==flags; }

	inline EntityID getID() const { return m_id; }
	
	virtual EntityType getEntityType() const = 0;

	inline const Phys::Vect& getPosition() const { return m_pos; }
	inline Phys::Vect& getPosition() { return m_pos; }

	virtual void update(World* w) {};

	void save(NBT& src) override;
	void load(NBT& src) override;

#ifdef ND_DEBUG
	inline virtual std::string toString() const { return "UNDEFINED_ENTITY"; }

#define TO_ENTITY_STRING(x)\
	inline std::string toString() const override {return #x;}
#else
#define TO_STRING(X)
#endif

};

