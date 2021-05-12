#include "ItemTool.h"


#include "core/AppGlobals.h"
#include "audio/Player.h"
#include "world/entity/EntityPlayer.h"

std::vector<glm::vec3> ItemTool::s_dug_blocks;

// number of blocks that can be dug simultaneously
constexpr int ITEM_TOOL_DUG_BLOCKS_MAX_SIZE = 20;

// the mass that needs to be removed to dig that block
// -1 means block has not been dug yet
static int getDiggingIndex(int x, int y)
{
	for (int i = 0; i < ItemTool::getDugBlocks().size(); i++) {
		auto& block = ItemTool::getDugBlocks()[i];
		if (x == block.x && y == block.y)
			return i;
	}
	return -1;
}

static glm::vec3& getDiggingBlock(int x, int y)
{
	auto& blocks = ItemTool::getDugBlocks();
	for (int i = 0; i < blocks.size(); i++) {
		auto& block = blocks[i];
		if (x == block.x && y == block.y)
			return block;
	}
	if (blocks.size() == ITEM_TOOL_DUG_BLOCKS_MAX_SIZE)
		blocks.erase(blocks.end() - 1);

	blocks.emplace_back(glm::vec3(x, y, -1));
	return blocks[blocks.size() - 1];
}

ItemTool::ItemTool(ItemID id, const std::string& textName, ToolType type)
	:Item(id, textName), m_tool_type(type)
{
	m_max_stack_size = 1;
}

struct ItemToolDataBox
{
	int ticksForNextSwing;
	int blockX, blockY;
};
void* ItemTool::instantiateDataBox() const
{
	return new ItemToolDataBox();
}

void ItemTool::destroyDataBox(void* dataBox) const
{
	delete (ItemToolDataBox*)dataBox;
}

void ItemTool::onEquipped(World& world, ItemStack& stack, WorldEntity& owner) const
{
	getDugBlocks().clear();
}

void ItemTool::onInteraction(World& w, ItemStack& stack, void* dataBox, WorldEntity& owner, float x, float y, Interaction interaction, int ticksPressed) const
{
	auto newPos = EntityPlayer::pickBlockToDig(w, owner.getNPos(), glm::vec2(x, y), 6);
	if (newPos.x != -1)
		AppGlobals::get().nbt["diggingPos"] = newPos;
	else 
		AppGlobals::get().nbt["diggingPos"] = NBT();
	
	//for (auto pos : EntityPlayer::pickBlocksToDig(w, owner.getNPos(), glm::vec2(x, y), 6))
	//	AppGlobals::get().nbt["diggingPos"].push_back(pos);

	auto& player = dynamic_cast<EntityPlayer&>(owner);
	auto data = (ItemToolDataBox*)dataBox;

	if (ticksPressed == 0) {
		player.setItemSwinging(true);
		//data->blockX = x;
		//data->blockY = y;
		data->ticksForNextSwing = 0;
	}
	else if (interaction == Interaction::RELEASED)
	{
		data->ticksForNextSwing = 0;
		player.setItemSwinging(false);
		AppGlobals::get().nbt["diggingPos"] = NBT();
		return;
	}
	//decrease counter for next swing
	data->ticksForNextSwing--;
	if (data->ticksForNextSwing < 0)
		data->ticksForNextSwing = 0;

	player.setFacingDir(player.getFacingDirection().x < 0);

	if (newPos.x != -1)
	{
		x = newPos.x;
		y = newPos.y;
	}
	else {
		data->ticksForNextSwing = 0;
		//player.setItemSwinging(false);
		return;
	}

	auto structInWorld = w.getBlockM(x, y);

	if (structInWorld->isAir())
		return;

	auto& block = BlockRegistry::get().getBlock(structInWorld->block_id);
	auto efficiency = getEfficiencyOnBlock(block, &stack);//60 ticks will take to dig the block

	if (data->ticksForNextSwing == 0) {//we haven't started digging yet
		data->ticksForNextSwing = m_dig_interval;

		{//spawn particles and update block cracks
			w.spawnBlockBreakParticles(x, y, 4);
			//flip cracks
			structInWorld->block_corner ^= BLOCK_STATE_CRACKED;
			// mark chunk dirty for graphical update
			w.getChunkM(World::toChunkCoord(x), World::toChunkCoord(y))->markDirty(true);
		}
		{
			auto audioPath = ND_RESLOC("res/audio/dig_stone/dig_") + std::to_string(std::rand()%4)+".ogg";
			//play dig sound
			Sounder::get().playSound(audioPath,0.5f);
		}


		auto& state = getDiggingBlock(x, y);
		if (state.z == -1.f)
			state.z = block.getHardness();

		state.z -= efficiency;
		if (state.z <= 0.f)//block is finished
		{
			if (block.getHardness() == 0.f)//insta dig means no waiting for another swing
				data->ticksForNextSwing = 2;

			ItemTool::getDugBlocks().erase(ItemTool::getDugBlocks().begin() + getDiggingIndex(x, y));
			auto t = block.createItemStackFromBlock(*structInWorld);
			if (t == nullptr)
			{
				ASSERT(false, "Cannot spawn item from block: {}", block.getStringID());
				return;
			}
			auto itemEntity = (EntityItem*)EntityAllocator::createEntity(ENTITY_TYPE_ITEM);
			itemEntity->setItemStack(t);
			itemEntity->getPosition() = glm::vec2((int)x + 0.5f, (int)y + 0.5f);
			itemEntity->getVelocity() = { 0, 5 / 60.f };
			w.spawnBlockBreakParticles(x, y);
			w.setBlockWithNotify(x, y, 0);
			w.spawnEntity(itemEntity);
		}

	}
}

float ItemTool::getEfficiencyOnBlock(const Block& block, ItemStack* stack) const
{
	//if block is too hard to dig out
	if (block.getTier() > m_tier)
		return 0;

	return m_efficiency;

}



