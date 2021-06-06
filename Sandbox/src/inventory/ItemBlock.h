#pragma once
#include "Item.h"
#include "world/block/Block.h"


class ItemBlock:public Item
{
protected:
	int m_block_id;
	//texture is either in item/... or in item_block/...
	bool m_no_block_texture=false;
public:
	ItemBlock(ItemID id,BlockID blockID, const std::string& name,int maxTextureMetadata=0);
	inline ItemBlock& setNoBlockTexture(bool noBlockTexture) { m_no_block_texture = noBlockTexture; return *this; }
	void onTextureLoaded(const nd::TextureAtlas& atlas) override;
	int getTextureOffset(const ItemStack& b) const override;

	int getBlockMetadata(ItemStack* stack)const;
	int getBlockID() const override;

	void* instantiateDataBox() const override;
	void destroyDataBox(void* dataBox) const override;

	void onInteraction(World& w, ItemStack& stack, void* dataBox, WorldEntity& owner, float x, float y, Interaction interaction, int ticksPressed) const override;
};
