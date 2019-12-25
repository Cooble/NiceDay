#include "EntityPlayer.h"
#include "graphics/API/Texture.h"
#include "core/Stats.h"
//PlayerEntity======================================================


EntityPlayer::EntityPlayer()
{
	m_is_item_consumer = true;
	m_velocity = vec2(0.0f);
	m_acceleration = { 0.f, -9.8f / 60 };
	m_max_velocity = vec2(50.f / 60);

	static SpriteSheetResource res(Texture::create(
		TextureInfo("res/images/player3.png")
		.filterMode(TextureFilterMode::NEAREST)
		.format(TextureFormat::RGBA)), 8, 1);
	m_animation = Animation(&res, { 1, 2, 3, 2, 1, 4, 5, 4 });
	m_animation.setSpriteFrame(0, 0);
	m_animation.setPosition({ -1, 0, 0 });
	m_animation.setSize({ 2, 3 });
	m_bound = Phys::toPolygon(Phys::Rectangle::createFromDimensions(-0.75f, 0, 1.5f, 2.9f));
	m_inventory.putAtRandomIndex(ItemStack::create(SID("pickaxe")));
	m_inventory.putAtRandomIndex(ItemStack::create(SID("pickaxe")));
	m_inventory.putAtRandomIndex(ItemStack::create(SID("stone"),7));
	m_inventory.putAtRandomIndex(ItemStack::create(SID("stone"),5));
}

void EntityPlayer::update(World& w)
{
	auto lastPos = m_pos;
	if (!Stats::move_through_blocks_enable)
		PhysEntity::computePhysics(w);
	else
	{
		m_velocity = glm::clamp(m_velocity, -m_max_velocity, m_max_velocity);
		m_pos += m_velocity;
	}

	if (this->m_pos.x > lastPos.x)
	{
		m_animation.setHorizontalFlip(false);
		m_animation_var = 1;
	}
	else if (this->m_pos.x < lastPos.x)
	{
		m_animation.setHorizontalFlip(true);
		m_animation_var = 1;
	}
	else
	{
		if (this->m_pos.y != lastPos.y)
		{
			m_animation_var = 0;
			m_animation.setHorizontalFlip(false);
			m_animation.setSpriteFrame(0, 0);
			m_pose = 0;
			m_last_pose = 0;
		}
		else
		{
			if (m_animation_var == 1)
				m_animation.setSpriteFrame(1, 0);
		}
	}
	constexpr float animationBlocksPerFrame = 1;
	m_pose += abs(lastPos.x - m_pos.x);
	if (m_pose - m_last_pose > animationBlocksPerFrame)
	{
		m_pose = m_last_pose;
		m_animation.nextFrame();
	}
}


EntityType EntityPlayer::getEntityType() const
{
	return ENTITY_TYPE_PLAYER;
}

bool EntityPlayer::wantsItem(const ItemStack* stack) const
{
	return m_inventory.isSpaceFor(stack);
}

ItemStack* EntityPlayer::consumeItem(ItemStack* stack)
{
	return m_inventory.putAtRandomIndex(stack);
}

void EntityPlayer::onHit(World& w, WorldEntity* e, float damage)
{
}

void EntityPlayer::save(NBT& src)
{
	Creature::save(src);
	src.set("playerName", m_name);
	NBT t;
	m_inventory.save(t);
	src.set("inventory", t);
}

void EntityPlayer::load(NBT& src)
{
	Creature::load(src);
	m_name = src.get<std::string>("playerName", "Karel");
	m_inventory.load(src.get<NBT>("inventory"));
}