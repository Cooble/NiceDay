#include "CreativeInventory.h"

CreativeInventory::CreativeInventory()
{
	m_id = "creative";
	auto& items = ItemRegistry::get().getItems();
	setInventorySize(items.size());
	int index = 0;
	for (auto item : items)
		m_items[index++] = ItemStack::create(item.first);
}

ItemStack* CreativeInventory::takeFromIndex(int index, int number)
{
	auto out = m_items[index]->copy();
	if (number > 0)
		out->setSize(number);
	return out;
}


ItemStack* CreativeInventory::putAtRandomIndex(ItemStack* stack)
{
	return stack;
}

ItemStack* CreativeInventory::putAtIndex(ItemStack* stack, int index, int count)
{
	return stack;
}

ItemStack* CreativeInventory::swap(ItemStack* stack, int index)
{
	return stack;
}

bool CreativeInventory::isSpaceFor(const ItemStack* stack) const
{
	return false;
}
