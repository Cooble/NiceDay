#pragma once
#include "ndpch.h"

constexpr int ENTITY_BOULDER = 0;

class World;

class WorldEntity
{
private:
	int m_id;// each entity instance has unique id
	int m_entity_type;// e.g. Player, Zombie, Skeleton etc.
protected:
	bool m_is_temporary;//should be destroyed or saved upon unload
	bool m_is_chunk_loader;//can keep chunks loaded
	Vector2D m_pos;//world pos
	WorldEntity(int id,int entityTypeId);
public:
	virtual ~WorldEntity() = default;
	inline bool isChunkLoader() const { return m_is_chunk_loader; }//keeps chunks loaded
	inline bool isTemporary() const { return m_is_temporary; }//should be destroyed or saved upon unload
	inline int getID() const { return m_id; }
	inline int getEntityType() const { return m_entity_type; }
	inline const Vector2D& getPosition() const { return m_pos; }

	virtual void update(World* w);

};
