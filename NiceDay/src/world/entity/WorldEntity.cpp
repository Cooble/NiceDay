#include "ndpch.h"
#include "WorldEntity.h"

WorldEntity::WorldEntity(int id, int entityTypeId)
	:m_id(id),m_entity_type(entityTypeId)
{
}

void WorldEntity::update(World* w)
{
}
