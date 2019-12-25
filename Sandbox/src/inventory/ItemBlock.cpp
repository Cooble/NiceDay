#include "ItemBlock.h"
#include "graphics/TextureAtlas.h"
#include "world/block/BlockRegistry.h"

ItemBlock::ItemBlock(ItemID id, BlockID blockID, const std::string& name) :
Item(id, name),
m_block_id(0)
{
}

void ItemBlock::onTextureLoaded(const TextureAtlas& atlas)
{
	m_texture_pos = atlas.getTexture("item_block/" + toString());
}

bool ItemBlock::canBePlaced(World& w, WorldEntity& player, int x, int y)
{
	return BlockRegistry::get().getBlock(m_block_id).canBePlaced(w, x, y);
}

int ItemBlock::getBlockID() const
{
	return m_block_id;
}
