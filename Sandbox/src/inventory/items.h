#pragma once
#include "Item.h"
#include "ItemTool.h"

class ItemMagicWand :public Item
{
public:
   ItemMagicWand();
	bool onRightClickOnBlock(World& world, ItemStack& stack, WorldEntity& owner, int x, int y, BlockStruct& block) const override;
};
class ItemHammer :public ItemTool
{
public:
   ItemHammer(ItemID id,const std::string& name);
	void onItemInteraction(World& w, ItemStack& stack, void* dataBox, WorldEntity& owner, float x, float y, Interaction interaction, int ticksPressed) const override;
	bool onRightClickOnBlock(World& world, ItemStack& stack, WorldEntity& owner, int x, int y, BlockStruct& block) const override;
};
class ItemArmor :public Item
{
protected:
	friend ItemRegistry;
	int m_defense = 1;

public:
   ItemArmor(ItemID id,const std::string& name);

   int getDefense() const { return m_defense; }
   
};

class ItemShotgun : public Item
{
public:
   ItemShotgun();

   bool onRightClick(World& world, ItemStack& stack, WorldEntity& owner, int x, int y) const override;

   std::string getTitle(ItemStack* stack) const override;
};

class ItemTnt : public Item
{
public:
   ItemTnt();

   bool onRightClick(World& world, ItemStack& stack, WorldEntity& owner, int x, int y) const override;
};

class ItemVinyl : public Item
{
public:
   ItemVinyl();

   std::string getTitle(ItemStack* stack) const override;
   bool onRightClickOnBlock(World& world, ItemStack& stack, WorldEntity& owner, int x, int y, BlockStruct& block) const override;



};
