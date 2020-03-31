#include "items.h"
#include "world/block/block_datas.h"
#include "world/entity/EntityPlayer.h"

ItemPickaxe::ItemPickaxe()
	:ItemTool(SID("pickaxe"),"pickaxe",TOOL_TYPE_PICKAXE)
{
	m_maxStackSize = 1;
}

float ItemPickaxe::getEfficiencyOnBlock(const Block& blok, ItemStack* stack) const
{
	return 1;
}

ItemShotgun::ItemShotgun()
	:Item(SID("shotgun"), "shotgun")
{
	//m_has_nbt = true;
	m_maxStackSize = 1;
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
	m_maxStackSize = 111;
	m_max_texture_metadata = 3;
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

