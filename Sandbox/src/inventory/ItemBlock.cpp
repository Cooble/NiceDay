#include "ItemBlock.h"
#include "graphics/TextureAtlas.h"

ItemBlock::ItemBlock(ItemID id, const std::string& name): Item(id, name)
{
	m_is_block = true;
}

void ItemBlock::onTextureLoaded(const TextureAtlas& atlas)
{
	m_texture_pos = atlas.getTexture("item_block/" + toString());
}

int ItemBlock::getBlockID() const
{
	return m_block_id;
}
