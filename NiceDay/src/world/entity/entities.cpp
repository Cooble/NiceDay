#include "ndpch.h"
#include "entities.h"
#include "graphics/BatchRenderer2D.h"
#include "entity_datas.h"


void Creature::render(BatchRenderer2D& renderer)
{
	renderer.push(glm::translate(glm::mat4(1.0f), glm::vec3(m_pos.x, m_pos.y, 0)));
	renderer.submit(m_sprite);
	renderer.pop();
}

void Creature::save(NBT& src)
{
	src.set("health", m_stat_health);
}

void Creature::load(NBT& src)
{
	m_stat_health = src.get("health", 10.0f);
}

void EntityPlayer::update(World* w)
{
}


int EntityPlayer::getEntityType() const
{
	return ENTITY_TYPE_PLAYER;
}
void EntityPlayer::save(NBT& src)
{
	src.set("playerName", m_name);
}

void EntityPlayer::load(NBT& src)
{
	m_name = src.get<std::string>("playerName", "Karel");
}
