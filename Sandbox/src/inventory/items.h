#pragma once
#include "Item.h"
#include "ItemTool.h"

class ItemPickaxe:public ItemTool
{
public:
	ItemPickaxe();

	float getEfficiencyOnBlock(const Block& blok, ItemStack* stack) const override;
};

class ItemShotgun: public Item
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
