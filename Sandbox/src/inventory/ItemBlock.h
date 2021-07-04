#pragma once
#include "Item.h"
#include "world/block/Block.h"


class ItemBlock:public Item
{
protected:
	int m_block_id;
	//texture is either in item/... or in item_block/...
	bool m_custom_texture=false;
public:
	ItemBlock(ItemID id,BlockID blockID, const std::string& name,int maxTextureMetadata=0);
	inline ItemBlock& setCustomTexture(bool noBlockTexture) { m_custom_texture = noBlockTexture; return *this; }
	void onTextureLoaded(const nd::TextureAtlas& atlas) override;
	int getTextureOffset(const ItemStack& b) const override;

	int getBlockMetadata(ItemStack* stack)const;
	int getBlockID() const override;

	void* instantiateDataBox() const override;
	void destroyDataBox(void* dataBox) const override;

	void onItemInteraction(World& w, ItemStack& stack, void* dataBox, WorldEntity& owner, float x, float y, Interaction interaction, int ticksPressed) const override;
};
class ItemWall:public Item
{
protected:
	int m_wall_id;
	//texture is either in item/... or in item_block/...
	bool m_custom_texture=false;
public:
	ItemWall(ItemID id,BlockID wallID, const std::string& name);
	inline ItemWall& setCustomTexture(bool noBlockTexture) { m_custom_texture = noBlockTexture; return *this; }
	void onTextureLoaded(const nd::TextureAtlas& atlas) override;
	int getTextureOffset(const ItemStack& b) const override;

	void* instantiateDataBox() const override;
	
	void destroyDataBox(void* dataBox) const override;

	void onItemInteraction(World& w, ItemStack& stack, void* dataBox, WorldEntity& owner, float x, float y, Interaction interaction, int ticksPressed) const override;
};
