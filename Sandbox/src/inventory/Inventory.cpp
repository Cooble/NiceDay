﻿#include "Inventory.h"
#include "ItemStack.h"

using namespace nd;

BasicInventory::~BasicInventory()
{
	for (auto item : m_items)
		if (item)item->destroy();
}

void BasicInventory::setInventorySize(int size)
{
	m_items.resize(size);
	ZeroMemory(m_items.data(), size * sizeof(ItemStack*));
}

ItemStack* BasicInventory::putAtRandomIndex(ItemStack* stack)
{
	auto& item = stack->getItem();

	int toAdd = stack->size();
	int lastNullFreeIndex = -1;
	bool isStackable = item.getMaxStackSize() > 1;
	for (int i = 0; i < m_items.size(); ++i)
	{
		auto currentStack = m_items[i];
		if (currentStack == nullptr && lastNullFreeIndex == -1)
			lastNullFreeIndex = i;

		if (currentStack == nullptr)
			continue;
		//skip trying to fit items which maxsize is only 1
		if (!isStackable)
			continue;

		if (stack->equals(currentStack))
		{
			int freeSpace = item.getMaxStackSize() - currentStack->size();
			if (freeSpace > 0)
			{
				int willAdd = std::min(freeSpace, toAdd);
				toAdd -= willAdd;
				currentStack->addSize(willAdd);
			}
		}
	}

	stack->setSize(toAdd);

	if (toAdd > 0)
	{
		//we are full
		if (lastNullFreeIndex == -1)
			return stack;

		//add to free slot
		m_items[lastNullFreeIndex] = stack;
	}
	else
		stack->destroy();

	return nullptr;
}

ItemStack* BasicInventory::putAtIndex(ItemStack* stack, int index, int count)
{
	auto& target = m_items[index];
	if (target != nullptr && stack->equals(target))
	{
		int freeSpace = target->getItem().getMaxStackSize() - target->size();
		int toAdd = std::min(freeSpace, count == ALL ? stack->size() : count);
		target->addSize(toAdd);
		stack->addSize(-toAdd);
		if (stack->isEmpty())
		{
			stack->destroy();
			return nullptr;
		}
	}
	else if (target == nullptr)
	{
		int toAdd = count == -1 ? stack->size() : count;
		if (toAdd == stack->size())
		{
			target = stack;
			return nullptr;
		}
		target = stack->copy();
		target->setSize(toAdd);
		stack->addSize(-toAdd);
	}
	return stack;
}

ItemStack* BasicInventory::swap(ItemStack* stack, int index)
{
	auto t = m_items[index];
	m_items[index] = stack;
	return t;
}

ItemStack* BasicInventory::getItemStack(int index)
{
	return m_items[index];
}

const std::string& BasicInventory::getID() const
{
	return m_id;
}

int BasicInventory::getItemsSize() const
{
	return m_items.size();
}

ItemStack* BasicInventory::takeFromIndex(int index, int number)
{
	auto& target = m_items[index];
	if (target == nullptr)
		return nullptr;
	if (target->size() <= number || number == ALL)
	{
		auto out = target;
		target = nullptr;
		return out;
	}
	else
	{
		auto out = target->copy();
		target->addSize(-number);
		out->setSize(number);
		return out;
	}
}

void BasicInventory::save(NBT& src)
{
	src.save("size",m_items.size());
	NBT list;
	for (int i = 0; i < m_items.size(); ++i)
	{
		if (m_items[i] != nullptr)
		{
			NBT t;
			t["slotter"] = i;
			m_items[i]->serialize(t);
			list.push_back(std::move(t));
		}
	}
	src["slots"] = std::move(list);
}

void BasicInventory::load(NBT& src)
{
	m_items.clear();
	NBT& list = src["slots"];
	m_items.resize(src["size"]);
	
	ZeroMemory(m_items.data(), m_items.size() * sizeof(ItemStack*));
	for (auto value : list.arrays())
		m_items[value["slotter"]] = ItemStack::deserialize(value);
}