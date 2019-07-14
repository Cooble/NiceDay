#include "ndpch.h"
#include "WorldEntity.h"


void WorldEntity::save(NBT& src)
{
	src.set("entityID",m_id);
}

void WorldEntity::load(NBT& src)
{
	m_id = src.get<EntityID>("entityID");
}
