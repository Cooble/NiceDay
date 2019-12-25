#include "ndpch.h"
#include "WorldEntity.h"
#include "world/World.h"


void WorldEntity::save(NBT& src)
{
	src.set("entityPos", m_pos);
	src.set("entityTypeID", getEntityType());
	src.set("entityID",m_id);
}

void WorldEntity::load(NBT& src)
{
	m_id = src.get<EntityID>("entityID");
	m_pos = src.get<glm::vec2>("entityPos");
}
//TileEntity====================================================

TileEntity::TileEntity()
	: m_last_update_ticks(0),
	m_age(0)
{
}


void TileEntity::update(World& w)
{
	auto newTime = w.getWorldTime();
	m_age += (newTime - m_last_update_ticks);
	m_last_update_ticks = newTime;
}

void TileEntity::onSpawned(World& w)
{
	m_last_update_ticks = w.getWorldTime();
	m_age = 0;
}

void TileEntity::onClicked(World& w, WorldEntity* entity)
{
}

BlockStruct& TileEntity::getBlockStruct(World& w)
{
	return *w.getBlockM(m_pos.x, m_pos.y);
}

void TileEntity::save(NBT& src)
{
	WorldEntity::save(src);
	src.set("lastUpdateT", m_last_update_ticks.m_ticks);
	src.set("age", m_age.m_ticks);
}

void TileEntity::load(NBT& src)
{
	WorldEntity::load(src);
	m_last_update_ticks = src.get<long long>("lastUpdateT", 0);
	m_age = src.get<long long>("age", 0);
}
