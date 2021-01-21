#pragma once
#include "inventory/Inventory.h"
#include "world/entity/EntityAllocator.h"

enum InventorySlot :int
{
	HAND = 0,
	ARMOR_HELMET = 1,
	ARMOR_CHEST = 2,
	ARMOR_LEGGINS = 3,
	ARMOR_BOOTS = 4,

	ACTION_FIRST = 5,
	RANDOM_FIRST = ACTION_FIRST + 10,
	
};

class PlayerInventory:public Inventory
{
private:
	int m_special_hand_slot;
	std::vector<ItemStack*> m_items;
	WorldEntity* m_player;
public:
	PlayerInventory(WorldEntity* player);
	int trashSlot() { return m_items.size() - 1; }
	void callEquipped(ItemStack* itemStack);
	void callUnequipped(ItemStack* itemStack);
	ItemStack* putAtRandomIndex(ItemStack* stack) override;
	ItemStack* putAtIndex(ItemStack* stack, int index,int count=-1) override;
	ItemStack* swap(ItemStack* stack, int index) override;
	ItemStack* getItemStack(int index) override;
	const std::string& getID() const override;
	int getItemsSize() const override;
	ItemStack* takeFromIndex(int index, int number) override;
	/**
	 * returns item that player is currently holding (by cursor or in selected itemslot)
	 */
	ItemStack*& itemInHand();
	int itemInHandSlot()const;
	/**
	 * returns temporary handslot item
	 */
	ItemStack*& handSlot();
	
	bool isSpaceFor(const ItemStack* stack) const override;
	void setHandIndex(int index);
	int getHandIndex();

	void save(NBT& src);
	void load(NBT& src);
};
