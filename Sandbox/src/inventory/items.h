﻿#pragma once
#include "Item.h"
#include "ItemTool.h"

class ItemPickaxeCopper :public ItemTool
{
public:
   ItemPickaxeCopper();
};
class ItemElPickaxo :public ItemTool
{
public:
   ItemElPickaxo();
};
class ItemMagicWand :public Item
{
public:
   ItemMagicWand();
	bool onRightClickOnBlock(World& world, ItemStack& stack, WorldEntity& owner, int x, int y, BlockStruct& block) const override;
};
class ItemHammer :public ItemTool
{
public:
   ItemHammer();
	void onItemInteraction(World& w, ItemStack& stack, void* dataBox, WorldEntity& owner, float x, float y, Interaction interaction, int ticksPressed) const override;
	bool onRightClickOnBlock(World& world, ItemStack& stack, WorldEntity& owner, int x, int y, BlockStruct& block) const override;
};

class ItemWoodHelmet :public Item
{
public:
   ItemWoodHelmet();
};
class ItemIronHelmet :public Item
{
public:
   ItemIronHelmet();
};
class ItemWoodChestplate :public Item
{
public:
   ItemWoodChestplate();
};
class ItemWoodLeggins :public Item
{
public:
   ItemWoodLeggins();
};
class ItemWoodBoots :public Item
{
public:
   ItemWoodBoots();
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
