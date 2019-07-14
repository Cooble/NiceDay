#pragma once

#include "world/entity/WorldEntity.h"
#include "world/entity/EntityRegistry.h"

class Creature :public WorldEntity, IBatchRenderable2D
{
protected:
	float m_stat_health;
	Sprite m_sprite;

public:
	Creature() = default;

	void render(BatchRenderer2D& renderer) override;

	inline Sprite& getSprite() { return m_sprite; }

	void save(NBT& src) override;
	void load(NBT& src) override;

};
class EntityPlayer :public Creature
{
private:
	std::string m_name;

public:
	EntityPlayer() = default;

	void update(World* w) override;
	int getEntityType() const override;
	glm::vec2& getPosition() { return m_pos; }

	ND_FACTORY_METH_ENTITY_BUILD(EntityPlayer)

	void save(NBT& src) override;
	void load(NBT& src) override;

};