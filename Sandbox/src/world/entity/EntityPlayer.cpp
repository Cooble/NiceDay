#include "EntityPlayer.h"
#include "graphics/API/Texture.h"
#include "core/Stats.h"
//PlayerEntity======================================================


EntityPlayer::EntityPlayer()
	:m_inventory(this)
{
	m_is_item_consumer = true;
	m_velocity = vec2(0.0f);
	m_acceleration = { 0.f, -9.8f / 60 };
	m_max_velocity = vec2(40.f / 60);

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
	m_inventory.putAtRandomIndex(ItemStack::create(SID("shotgun")));
}

void EntityPlayer::render(BatchRenderer2D& renderer)
{
	constexpr float ITEM_ATLAS_SIZE_BIT = 1.f/16;
	
	renderer.push(glm::translate(glm::mat4(1.0f), glm::vec3(m_pos.x, m_pos.y, 0)));
	renderer.submit(m_animation);

	//item
	auto stack = getInventory().itemInHand();
	if (stack&&m_is_swinging) {
		auto& item = stack->getItem();

		half_int txtOffset = item.getTextureOffset(*stack);

		auto m = glm::translate(glm::mat4(1.f), { 0, 1, 0 });
		m = glm::rotate(m, Phys::toRad(m_item_angle), { 0, 0, 1 });
		m = glm::translate(m, { 0, 0.3f, 0 });
		renderer.push(m);

		static auto textureAtlas = Texture::create(
			TextureInfo("res/images/itemAtlas/atlas.png").
			filterMode(TextureFilterMode::NEAREST));
		renderer.submitTextureQuad(
			{ 0,0,0 },
			{ 2,2 },
			UVQuad::build({ txtOffset.x * ITEM_ATLAS_SIZE_BIT,txtOffset.y * ITEM_ATLAS_SIZE_BIT }, { ITEM_ATLAS_SIZE_BIT ,ITEM_ATLAS_SIZE_BIT }), textureAtlas);
		renderer.pop();
	}

	m_health_bar.setValue(m_health / m_max_health);
	m_health_bar.setEnabled(m_health < m_max_health);
	m_health_bar.render(renderer);

	renderer.pop();
	
	

	
}

void EntityPlayer::setItemSwinging(bool swing)
{
	if(!swing&&m_is_swinging&&!m_is_last_swing)
	{
		m_is_last_swing = true;
		m_item_angle = (int)m_item_angle % 360;
	}
	if (!m_is_swinging && swing) {
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
		m_item_angle += m_is_facing_left?swingSpeed:-swingSpeed;
	if(m_is_last_swing)
		if(m_item_angle>=360||m_item_angle<=-360)
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
		m_velocity = glm::clamp(m_velocity, -m_max_velocity, m_max_velocity);
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
	if (lastFacingLeft != m_is_facing_left)
		m_animation.updateAfterFlip();
	
	constexpr float animationBlocksPerFrame = 1;
	m_pose += abs(lastPos.x - m_pos.x);
	if (m_pose - m_last_pose > animationBlocksPerFrame)
	{
		m_pose = m_last_pose;
		if (!w.isAir(m_pos.x, m_pos.y - 1.f) || lastFacingLeft!=m_is_facing_left|| Stats::move_through_blocks_enable)//no walking while jumping through air
			m_animation.nextFrame();
	}


	//call item update each tick as long as its held in hand
	ItemStack* item = m_inventory.itemInHand();
	if (item)
		item->getItem().onItemHeldInHand(w, *this, *item);
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
	src.save("playerName", m_name);
	NBT t;
	m_inventory.save(t);
	src["inventory"] = std::move(t);
}

void EntityPlayer::load(NBT& src)
{
	Creature::load(src);
	src.load("playerName", m_name,std::string("Karel"));
	m_inventory.load(src["inventory"]);
}