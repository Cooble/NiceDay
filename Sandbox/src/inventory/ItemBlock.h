#pragma once
#include "Item.h"
#include "world/block/Block.h"


class ItemBlock:public Item
{
protected:
	int m_block_id;
	bool m_shift_with_meta;
public:
	ItemBlock(ItemID id,BlockID blockID, const std::string& name,bool shiftTextureWithMeta=false);
	
	void onTextureLoaded(const TextureAtlas& atlas) override;
	int getTextureOffset(const ItemStack& b) const override;

	int getBlockMetadata(ItemStack* stack)const;
	int getBlockID() const override;
};
