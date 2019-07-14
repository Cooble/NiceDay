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
protected:
	EntityID m_id;// each entity instance has unique id
protected:
	uint64_t m_flags;
	glm::vec2 m_pos;//world pos
protected:
	WorldEntity() = default;
	virtual ~WorldEntity() = default;

public:
	inline bool hasFlag(uint64_t flags) const { return (m_flags & flags)==flags; }

	inline EntityID getID() const { return m_id; }
	
	virtual int getEntityType() const = 0;

	inline const glm::vec2& getPosition() const { return m_pos; }

	virtual void update(World* w) {};

	void save(NBT& src) override;
	void load(NBT& src) override;

};

