#include "ndpch.h"
#include "WorldEntity.h"
#include "world/World.h"
#include "core/NBT.h"



void WorldEntity::save(NBT& src)
{
	src.save("entityPos", m_pos);
	src.save("entityTypeID", getEntityType());
	src.save("entityID", m_id);
}

void WorldEntity::load(NBT& src)
{
	src.load("entityPos", m_pos);
	src.load("entityID", m_id);
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
	src.save("lastUpdateT", m_last_update_ticks.m_ticks);
	src.save("age", m_age.m_ticks);
}

void TileEntity::load(NBT& src)
{
	WorldEntity::load(src);
	src.load("lastUpdateT", m_last_update_ticks.m_ticks);
	src.load("age", m_age.m_ticks);
}
