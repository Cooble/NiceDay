#pragma once
#include "Item.h"
#include "world/block/Block.h"


class ItemBlock:public Item
{
protected:
	int m_block_id;
public:
	ItemBlock(ItemID id,BlockID blockID, const std::string& name);
	void onTextureLoaded(const TextureAtlas& atlas) override;
	bool canBePlaced(World& w, WorldEntity& player, int x, int y);
	int getBlockID() const override;
};
