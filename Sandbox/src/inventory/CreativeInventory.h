#pragma once
#include "Inventory.h"

class CreativeInventory:public BasicInventory
{
public:
	CreativeInventory();
	ItemStack* takeFromIndex(int index, int number) override;
	ItemStack* putAtRandomIndex(ItemStack* stack) override;
	ItemStack* putAtIndex(ItemStack* stack, int index, int count) override;
	ItemStack* swap(ItemStack* stack, int index) override;
	bool isSpaceFor(const ItemStack* stack) const override;
};
