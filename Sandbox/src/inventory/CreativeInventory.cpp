#include "CreativeInventory.h"

CreativeInventory::CreativeInventory()
{
	m_id = "creative";
	auto& items = ItemRegistry::get().getItems();
	for (auto item : items) {
		for (int i = 0; i < item.second->getMaxTextureMeta(); ++i)
		{
			auto t = ItemStack::create(item.first,Item::INFINITE_SIZE);
			t->setMetadata(i);
			m_items.push_back(t);
		}
		if(!item.second->getMaxTextureMeta())
			m_items.push_back(ItemStack::create(item.first, Item::INFINITE_SIZE));
	}
	//setInventorySize(m_items.size());
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
