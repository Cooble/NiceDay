#include "ItemBlock.h"
#include "graphics/TextureAtlas.h"
#include "world/block/BlockRegistry.h"

ItemBlock::ItemBlock(ItemID id, BlockID blockID, const std::string& name,bool shiftTextureWithMeta) :
Item(id, name),
m_block_id(blockID),
m_shift_with_meta(shiftTextureWithMeta)
{
	m_is_block = true;
}

void ItemBlock::onTextureLoaded(const TextureAtlas& atlas)
{
	m_texture_pos = atlas.getTexture((m_no_block_texture?"item/":"item_block/") + toString());
}

int ItemBlock::getTextureOffset(const ItemStack& b) const
{
	return m_texture_pos + (m_shift_with_meta?half_int(b.getMetadata(), 0):0);
}

int ItemBlock::getBlockMetadata(ItemStack* stack) const
{
	return stack->getMetadata();
}

int ItemBlock::getBlockID() const
{
	return m_block_id;
}
