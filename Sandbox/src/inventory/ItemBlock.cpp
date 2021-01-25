#include "ItemBlock.h"
#include "graphics/TextureAtlas.h"
#include "world/block/BlockRegistry.h"
#include "world/World.h"

ItemBlock::ItemBlock(ItemID id, BlockID blockID, const std::string& name, int maxTextureMetadata) :
	Item(id, name),
	m_block_id(blockID)
{
	setFlag(ITEM_FLAG_USE_META_AS_TEXTURE,(bool)maxTextureMetadata);
	m_max_metadata = maxTextureMetadata;
	setFlag(ITEM_FLAG_IS_BLOCK);
}

void ItemBlock::onTextureLoaded(const TextureAtlas& atlas)
{
	m_texture_pos = atlas.getSubImage((m_no_block_texture ? "item/" : "item_block/") + toString());
}

int ItemBlock::getTextureOffset(const ItemStack& b) const
{
	return m_texture_pos + (isUseMetaAsTexture() ? half_int(b.getMetadata(), 0) : 0);
}

int ItemBlock::getBlockMetadata(ItemStack* stack) const
{
	return stack->getMetadata();
}

int ItemBlock::getBlockID() const
{
	return m_block_id;
}

// block can be placed each n ticks
constexpr int BLOCK_PLACE_COOLDOWN=5;
struct ItemBlockDataBox
{
	// time when last block was placed
	int lastPlaceTicks=0;
	
};
void* ItemBlock::instantiateDataBox() const
{
	return new ItemBlockDataBox();
}

void ItemBlock::destroyDataBox(void* dataBox) const
{
	delete (ItemBlockDataBox*)dataBox;
}


void ItemBlock::onInteraction(World& w, ItemStack& stack, void* dataBox, WorldEntity& owner, float x, float y, Interaction interaction, int ticksPressed) const
{
	auto data = (ItemBlockDataBox*)dataBox;

	// reset cooldown if released
	if (interaction == RELEASED)
		data->lastPlaceTicks = 0;
	
	if (interaction == Interaction::RELEASED)
		return;
	
	auto block = w.getBlock(x, y);

	if (block == nullptr) //invalid coords
		return;
	auto& blockInstance = BlockRegistry::get().getBlock(block->block_id);
	
	if (!BlockRegistry::get().getBlock(m_block_id).canBePlaced(w, x, y))
		return;

	// if too early for another block placement 
	if (w.getWorldTime().ticks() - data->lastPlaceTicks < BLOCK_PLACE_COOLDOWN)
		return;
	data->lastPlaceTicks = w.getWorldTime().ticks();

	// destroy block that is replaceable
	if (blockInstance.isReplaceable() && !block->isAir())
		w.spawnBlockBreakParticles(x, y);
	

	BlockStruct str = {};
	str.block_id = (short)m_block_id;
	str.block_metadata = getBlockMetadata(&stack);
	w.setBlockWithNotify(x, y, str);

	stack.addSize(-1);
}
