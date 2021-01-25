#include "items.h"
#include "world/block/block_datas.h"
#include "world/entity/EntityPlayer.h"

ItemPickaxeCopper::ItemPickaxeCopper()
	:ItemTool(SID("pickaxe"),"pickaxe",TOOL_TYPE_PICKAXE)
{
	m_tier = 1;
	m_efficiency = 0.5f;
}

ItemElPickaxo::ItemElPickaxo()
	:ItemTool(SID("el_pickaxo"), "el_pickaxo", TOOL_TYPE_PICKAXE)
{
	m_tier = 10000;
	m_efficiency =100.f;
	m_dig_interval = 1;
}

ItemWoodHelmet::ItemWoodHelmet()
	:Item(SID("wood_helmet"), "wood_helmet")
{
	setFlag(ITEM_FLAG_ARMOR_HEAD);
	m_max_stack_size = 1;
}

ItemIronHelmet::ItemIronHelmet()
	:Item(SID("iron_helmet"), "iron_helmet")
{
	setFlag(ITEM_FLAG_ARMOR_HEAD);
	m_max_stack_size = 1;
}

ItemWoodChestplate::ItemWoodChestplate()
	: Item(SID("wood_chestplate"), "wood_chestplate")
{
	setFlag(ITEM_FLAG_ARMOR_CHEST);
	m_max_stack_size = 1;

}

ItemWoodLeggins::ItemWoodLeggins()
	: Item(SID("wood_leggins"), "wood_leggins")
{
	setFlag(ITEM_FLAG_ARMOR_LEGGINS);
	m_max_stack_size = 1;

}

ItemWoodBoots::ItemWoodBoots()
	: Item(SID("wood_boots"), "wood_boots")
{
	setFlag(ITEM_FLAG_ARMOR_BOOTS);
	m_max_stack_size = 1;
}

ItemShotgun::ItemShotgun()
	:Item(SID("shotgun"), "shotgun")
{
	//m_has_nbt = true;
	m_max_stack_size = 1;
}

bool ItemShotgun::onRightClick(World& world, ItemStack& stack, WorldEntity& owner, int x, int y) const
{

	float angle = Phys::angleRad(dynamic_cast<Creature&>(owner).getFacingDirection());
	const int BULLET_COUNT = 10;
	constexpr float spreadAngle = 0.35;
	constexpr float speed = 1.2;
	//ND_INFO("NAgle {}", glm::vec2(dynamic_cast<Creature&>(owner).getFacingDirection()).angleDegrees());
	
	for (int i = 0; i < BULLET_COUNT; ++i)
	{
		float ratio = i / (float)(BULLET_COUNT-1);
		
		auto bullet = (EntityRoundBullet*)EntityAllocator::createEntity(ENTITY_TYPE_ROUND_BULLET);
		bullet->getPosition() = owner.getPosition()+glm::vec2(0,1.5);
		bullet->fire(angle+((ratio-0.5f)*spreadAngle), speed);
		bullet->setOwner(owner.getID());
		world.spawnEntity(bullet);
	}
	
	return true;
}

std::string ItemShotgun::getTitle(ItemStack* stack) const
{
	return Font::colorize(Font::BLACK, Font::DARK_GREY) + "SuperShotgun"+Font::BLACK+"XXX";
}

ItemTnt::ItemTnt()
	:Item(SID("tnt"),"tnt")
{
	m_max_stack_size = 111;
	setFlag(ITEM_FLAG_USE_META_AS_TEXTURE);
	m_max_metadata = 3;
}

bool ItemTnt::onRightClick(World& world, ItemStack& stack, WorldEntity& owner, int x, int y) const
{
	stack.addSize(-1);
	auto tnt = (EntityBomb*)EntityAllocator::createEntity(ENTITY_TYPE_BOMB);
	tnt->getPosition() = owner.getPosition() + glm::vec2(0, 1.5);
	tnt->setBombType(stack.getMetadata() + 1 * 3, stack.getMetadata() == 2, stack.getMetadata());
	tnt->getVelocity() = glm::normalize(glm::vec2(x,y) - owner.getPosition()) * 0.9f;
	world.spawnEntity(tnt);
	return true;
}

static std::array<const char*,2 > vinylPlays = { "Neon", "Tower Clock"};
static std::array<const char*,2 > vinylPlaysPath = { "neon.ogg", "tower_clock.ogg"};

ItemVinyl::ItemVinyl()
	:Item(SID("vinyl"),"vinyl")
{
	m_max_stack_size = 1;
	setFlag(ITEM_FLAG_HAS_NBT);
	m_max_metadata = 2;
}

std::string ItemVinyl::getTitle(ItemStack* stack) const
{
	return Font::colorize(Font::BLACK, Font::YELLOW) + "Vinyl: " + vinylPlays[stack->getMetadata()] + Font::BLACK + "XXX";
}

bool ItemVinyl::onRightClickOnBlock(World& world, ItemStack& stack, WorldEntity& owner, int x, int y,
	BlockStruct& block) const
{
	if(world.getBlock(x,y)->block_id==BLOCK_RADIO)
	{
		
	}
	return true;
}


