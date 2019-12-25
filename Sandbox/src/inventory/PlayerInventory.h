#pragma once
#include "inventory/Inventory.h"

enum InventorySlot:int
{
	HAND=0,
	ARMOR_HELMET=1,
	ARMOR_CHEST=2,
	ARMOR_LEGGINS=3,
	ARMOR_BOOTS=4,

	INVENTORY_SLOT_FIRST=5,
};

class PlayerInventory:public Inventory,public NBTSaveable
{
private:
	std::vector<ItemStack*> m_items;
public:
	PlayerInventory();
	
	ItemStack* putAtRandomIndex(ItemStack* stack) override;
	ItemStack* putAtIndex(ItemStack* stack, int index,int count=-1) override;
	ItemStack* swap(ItemStack* stack, int index) override;
	ItemStack* getItemStack(int index) override;
	const std::string& getID() const override;
	int getItemsSize() const override;
	ItemStack* takeFromIndex(int index, int number) override;
	void save(NBT& src) override;
	void load(NBT& src) override;
	ItemStack*& itemInHand();
	bool isSpaceFor(const ItemStack* stack) const override;
};
