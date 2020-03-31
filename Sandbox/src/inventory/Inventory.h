#pragma once
#include "Item.h"

/**
 * Container methods always transfer ownership of itemstacks
 */
class Inventory
{
public:
	virtual ~Inventory() = default;
	/***
	 *tries to put item inside container
	 *@return null if success
	 *@return itemstack that was left after insertion
	 */
	virtual ItemStack* putAtRandomIndex(ItemStack* stack)=0;

	/***
	 *tries to put item in the slot
	 *@return null if success
	 *@return itemstack that was left after insertion
	 */
	virtual ItemStack* putAtIndex(ItemStack* stack, int index,int count=-1)=0;

	/***
	 * number -1 means all
	 */
	virtual ItemStack* takeFromIndex(int index,int number=-1)=0;
	virtual ItemStack* swap(ItemStack* stack, int index) = 0;
	virtual ItemStack* getItemStack(int index) = 0;
	virtual int getItemsSize() const = 0;
	virtual const std::string& getID() const = 0;
	virtual bool isSpaceFor(const ItemStack* stack) const { return true; };
	
};

class BasicInventory:public Inventory
{
protected:
	std::vector<ItemStack*> m_items;
	std::string m_id;
public:
	void setInventorySize(int size);
	inline void setInventoryID(const std::string& id) { m_id = id; }

	ItemStack* putAtRandomIndex(ItemStack* stack) override;
	ItemStack* putAtIndex(ItemStack* stack, int index, int count = -1) override;
	ItemStack* swap(ItemStack* stack, int index) override;
	ItemStack* getItemStack(int index) override;
	const std::string& getID() const override;
	int getItemsSize() const override;
	ItemStack* takeFromIndex(int index, int number) override;
	void save(NBT& src);
	void load(NBT& src);
};
