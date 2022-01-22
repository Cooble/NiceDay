﻿#include "EntityPlayer.h"
#include "graphics/API/Texture.h"
#include "core/Stats.h"
#include "inventory/ItemStack.h"
//PlayerEntity======================================================


using namespace glm;
using namespace nd;

EntityPlayer::EntityPlayer()
	: m_inventory(this)
{
	m_is_item_consumer = true;
	m_velocity = vec2(0.0f);
	m_acceleration = {0.f, -9.8f / 60 / 2};
	m_max_velocity = vec2(40.f / 60);

	static SpriteSheetResource res(Texture::create(
		                               TextureInfo("res/images/player3.png")
		                               .filterMode(TextureFilterMode::NEAREST)
		                               .format(TextureFormat::RGBA)), 8, 1);
	m_animation = Animation(&res, {1, 2, 3, 2, 1, 4, 5, 4});
	m_animation.setSpriteFrame(0, 0);
	m_animation.setPosition({-1, 0, 0});
	m_animation.setSize({2, 3});
	m_bound = Phys::toPolygon(Phys::Rectangle::createFromDimensions(-0.75f, 0, 1.5f, 2.9f));
	m_inventory.putAtRandomIndex(ItemStack::create(SID("el_pickaxo")));
	m_inventory.putAtRandomIndex(ItemStack::create(SID("pickaxe")));
	m_inventory.putAtRandomIndex(ItemStack::create(SID("stone"), 999));
	m_inventory.putAtRandomIndex(ItemStack::create(SID("stone"), -1));
	m_inventory.putAtRandomIndex(ItemStack::create(SID("shotgun")));
	m_inventory.putAtRandomIndex(ItemStack::create(SID("hammer")));
}

void EntityPlayer::render(BatchRenderer2D& renderer)
{
	constexpr float ITEM_ATLAS_SIZE_BIT = 1.f / 16;

	renderer.push(glm::translate(glm::mat4(1.0f), glm::vec3(m_pos.x, m_pos.y, 0)));
	renderer.submit(m_animation);

	//item
	auto stack = getInventory().itemInHand();
	if (stack && m_is_swinging)
	{
		auto& item = stack->getItem();

		half_int txtOffset = item.getTextureOffset(*stack);

		auto m = glm::translate(glm::mat4(1.f), {0, 1, 0});
		m = glm::rotate(m, Phys::toRad(m_item_angle), {0, 0, 1});
		m = glm::translate(m, {0, 0.3f, 0});
		renderer.push(m);

		static auto textureAtlas = Texture::create(
			TextureInfo("res/images/itemAtlas/atlas.png").
			filterMode(TextureFilterMode::NEAREST));
		renderer.submitTextureQuad(
			{0, 0, 0},
			{2, 2},
			UVQuad::build({txtOffset.x * ITEM_ATLAS_SIZE_BIT, txtOffset.y * ITEM_ATLAS_SIZE_BIT},
			              {ITEM_ATLAS_SIZE_BIT, ITEM_ATLAS_SIZE_BIT}), textureAtlas);
		renderer.pop();
	}

	m_health_bar.setValue(m_health / m_max_health);
	m_health_bar.setEnabled(m_health < m_max_health);
	m_health_bar.render(renderer);

	renderer.pop();
	if (m_bound.size() != 0 && Stats::show_collisionBox)
	{
		auto rect = m_bound.getBounds();
		auto mat = glm::translate(glm::mat4(1.0f), glm::vec3(m_pos.x + rect.x0, m_pos.y + rect.y0, 0));
		mat = glm::scale(mat, glm::vec3(rect.width(), rect.height(), 0));
		renderer.push(mat);
		renderer.submit(*Stats::bound_sprite);
		renderer.pop();
		if (getID() == ENTITY_TYPE_PLAYER)
		{
			/*auto& entityRectangle = m_bound.getBounds();
			for (int x = entityRectangle.x0-1; x < ceil(entityRectangle.x1)+1; ++x)
			{
				for (int y = entityRectangle.y0-1; y < ceil(entityRectangle.y1)+1; ++y)
				{
					glm::vec2 blockPos(x, y);
					blockPos += m_pos;

					auto stru = Stats::world->getBlockM((int)blockPos.x, (int)blockPos.y);
					if (stru == nullptr || stru->block_id == 0)
						continue;
					auto& block = BlockRegistry::get().getBlock(stru->block_id);
					if (!block.hasCollisionBox())
						continue;
					auto blockBounds = block.getCollisionBox((int)blockPos.x, (int)blockPos.y, *stru).copy();

					blockBounds.plus({ x, y });
					if (Phys::isIntersects(blockBounds, m_bound))
					{
						ND_INFO("we have render intersction");
					}
				}
			}*/


			for (int y = -4; y < 4; ++y)
			{
				for (int x = -4; x < 4; ++x)
				{
					const BlockStruct* str = Stats::world->getBlock(x + m_pos.x, y + m_pos.y);
					if (!str)
						continue;
					auto& b = BlockRegistry::get().getBlock(str->block_id);
					if (!b.hasCollisionBox())
						continue;
					auto polygon = b.getCollisionBox(x + m_pos.x, y + m_pos.y, *str);

					auto mat = glm::translate(glm::mat4(1.0f), glm::vec3((int)m_pos.x + x, (int)m_pos.y + y, 0));
					renderer.push(mat);
					auto firstVertex = polygon.list[0];
					for (int i = 1; i < polygon.list.size() - 1; i++)
						renderer.submitColorTriangle(firstVertex, polygon.list[i], polygon.list[i + 1], 0,
						                             glm::vec4(0, 1, 0, 0.85));
					renderer.pop();
				}
			}
		}
	}
}

void EntityPlayer::setItemSwinging(bool swing)
{
	if (!swing && m_is_swinging && !m_is_last_swing)
	{
		m_is_last_swing = true;
		m_item_angle = (int)m_item_angle % 360;
	}
	if (!m_is_swinging && swing)
	{
		m_item_angle = 0;
		m_is_swinging = true;
	}
}

void EntityPlayer::setFacingDir(bool left)
{
	m_is_facing_left = left;
	m_animation.setHorizontalFlip(left);
	m_animation_var = 1;
}


void EntityPlayer::update(World& w)
{
	constexpr float swingSpeed = 10;
	if (m_is_swinging)
		m_item_angle += m_is_facing_left ? swingSpeed : -swingSpeed;
	if (m_is_last_swing)
		if (m_item_angle >= 360 || m_item_angle <= -360)
		{
			m_is_swinging = false;
			m_is_last_swing = false;
		}

	auto lastPos = m_pos;
	auto lastFacingLeft = m_is_facing_left;
	if (!Stats::move_through_blocks_enable)
		PhysEntity::computePhysics(w);
	else
	{
		float len = glm::length(m_velocity);
		if (len > m_max_velocity.x)
			m_velocity = glm::normalize(m_velocity) * m_max_velocity.x;
		//m_velocity = glm::clamp(m_velocity, -m_max_velocity, m_max_velocity);
		m_pos += m_velocity;
	}

	if (this->m_pos.x > lastPos.x)
	{
		m_is_facing_left = false;
		m_animation.setHorizontalFlip(false);
		m_animation_var = 1;
	}
	else if (this->m_pos.x < lastPos.x)
	{
		m_animation.setHorizontalFlip(true);
		m_is_facing_left = true;
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
	//if (lastFacingLeft != m_is_facing_left)
	//	m_animation.updateAfterFlip();

	constexpr float animationBlocksPerFrame = 1;
	m_pose += abs(lastPos.x - m_pos.x);
	if (m_pose - m_last_pose > animationBlocksPerFrame)
	{
		m_pose = m_last_pose;
		if (!w.isAir(m_pos.x, m_pos.y - 1.f) || lastFacingLeft != m_is_facing_left || Stats::move_through_blocks_enable)
			//no walking while jumping through air
			m_animation.nextFrame();
	}


	//call item update each tick as long as its held in hand
	ItemStack* item = m_inventory.itemInHand();
	if (item)
		item->getItem().onItemHeldInHand(w, *this, *item);
}


EntityType EntityPlayer::getEntityType() const { return ENTITY_TYPE_PLAYER; }

bool EntityPlayer::wantsItem(const ItemStack* stack) const { return m_inventory.isSpaceFor(stack); }

ItemStack* EntityPlayer::consumeItem(ItemStack* stack) { return m_inventory.putAtRandomIndex(stack); }

void EntityPlayer::onHit(World& w, WorldEntity* e, float damage) {}

static bool isCornerWall(World& w,int x,int y)
{
	return
	w.isWallFree(x - 1, y)
	|| w.isWallFree(x + 1, y)
	|| w.isWallFree(x, y + 1)
	|| w.isWallFree(x, y - 1);
}
glm::ivec2 EntityPlayer::pickBlockToDig(World& w, glm::vec2 pos, glm::vec2 cursorPos, float radius, bool block_or_wall)
{
	std::vector<glm::ivec2> out;
	pos.y += 1.5f;
	glm::vec2 normal = glm::normalize(cursorPos - pos);

	for (int i = 0; i < glm::ceil(radius); ++i)
		for (int j = -1; j < 2; ++j)
		{
			for (int k = 0; k < 2; ++k)
			{
				glm::vec2 blockToPick = pos + glm::vec2(0.5f - k, j) + (float)i * normal;
				if (block_or_wall)
				{
					if (!w.isAir(blockToPick.x, blockToPick.y))
						return blockToPick;
				}
				else
				{
					auto b1 = w.getBlock(blockToPick.x, blockToPick.y);
					if (!w.isWallFree(blockToPick.x, blockToPick.y)
						&& (
							(b1->block_corner & BLOCK_STATE_PURE_MASK) != BLOCK_STATE_FULL
							|| !BlockRegistry::get().getBlock(b1->block_id).isSolid())
						&& isCornerWall(w,blockToPick.x, blockToPick.y))//wall has to be on the edge
						return blockToPick;
				}
			}
		}
	return glm::ivec2(-1, -1);
}


std::vector<glm::ivec2> EntityPlayer::pickBlocksToDig(World& w, glm::vec2 pos, glm::vec2 cursorPos, float radius)
{
	std::vector<glm::ivec2> out;
	pos.y += 1.5f;
	glm::vec2 normal = glm::normalize(cursorPos - pos);

	for (int i = 0; i < glm::ceil(radius); ++i)
		for (int j = -1; j < 2; ++j)
		{
			glm::vec2 blockToPick1 = pos + glm::vec2(0.5f, j) + (float)i * normal;
			glm::vec2 blockToPick2 = pos + glm::vec2(-0.5f, j) + (float)i * normal;

			out.emplace_back(glm::ivec2(blockToPick1));
			out.emplace_back(glm::ivec2(blockToPick2));
		}

	return out;
}

void EntityPlayer::save(NBT& src)
{
	Creature::save(src);
	src.save("playerName", m_name);
	NBT t;
	m_inventory.save(t);
	src["inventory"] = std::move(t);
}

void EntityPlayer::load(NBT& src)
{
	Creature::load(src);
	src.load("playerName", m_name, std::string("Karel"));
	m_inventory.load(src["inventory"]);
}
