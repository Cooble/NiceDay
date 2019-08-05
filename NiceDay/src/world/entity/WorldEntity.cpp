#include "ndpch.h"
#include "WorldEntity.h"


void WorldEntity::save(NBT& src)
{
	src.set("entityPos", m_pos);
	src.set("entityTypeID", getEntityType());
	src.set("entityID",m_id);
}

void WorldEntity::load(NBT& src)
{
	m_id = src.get<EntityID>("entityID");
	m_pos = src.get<Phys::Vect>("entityPos");
}
