#pragma once
#include "Item.h"


class ItemBlock:public Item
{
protected:
	int m_block_id;
public:
	ItemBlock(ItemID id, const std::string& name);
	void onTextureLoaded(const TextureAtlas& atlas) override;
	int getBlockID() const override;
};
