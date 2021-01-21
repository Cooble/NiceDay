#include "PlayerInventory.h"
#include "world/World.h"
#include "core/Stats.h"
#include "core/NBT.h"
#include <stack>

PlayerInventory::PlayerInventory(WorldEntity* player)
	: m_player(player)
{
	m_special_hand_slot = -1;
	constexpr int inventorySize = InventorySlot::ACTION_FIRST + 10 + 40;
	//9 activeslots + 27 random items

	m_items.resize(inventorySize);
	ZeroMemory(m_items.data(), inventorySize * sizeof(ItemStack*));
}

void PlayerInventory::callEquipped(ItemStack* itemStack)
{
	if (itemStack)
		itemStack->getItem().onEquipped(*Stats::world, *itemStack, *m_player);
}

void PlayerInventory::callUnequipped(ItemStack* itemStack)
{
	if (itemStack)
		itemStack->getItem().onUnequipped(*Stats::world, *itemStack, *m_player);
}

ItemStack* PlayerInventory::putAtRandomIndex(ItemStack* stack)
{
	auto& item = stack->getItem();

	int toAdd = stack->size();
	int lastNullFreeIndex = -1;
	bool isStackable = item.getMaxStackSize() > 1;
	for (int i = InventorySlot::ACTION_FIRST; i < m_items.size()-1; ++i)//dont put to trash
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
		if (lastNullFreeIndex == m_special_hand_slot)
			callEquipped(stack);
	}
	else
		stack->destroy();

	return nullptr;
}

ItemStack* PlayerInventory::putAtIndex(ItemStack* stack, int index, int count)
{
	auto& target = m_items[index];
	if (target != nullptr && stack->equals(target))
	{
		int freeSpace = target->getItem().getMaxStackSize() - target->size();
		int toAdd = std::min(freeSpace, count == -1 ? stack->size() : count);
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
			if (index == m_special_hand_slot)
				callEquipped(stack);
			else if (index == (int)InventorySlot::HAND)
			{
				if (m_special_hand_slot != -1)
				{
					callUnequipped(m_items[m_special_hand_slot]);
				}
				callEquipped(stack);
			}

			return nullptr;
		}
		target = stack->copy();
		target->setSize(toAdd);
		if (index == m_special_hand_slot)
			callEquipped(target);
		else if (index == (int)InventorySlot::HAND)
		{
			if (m_special_hand_slot != -1)
			{
				callUnequipped(m_items[m_special_hand_slot]);
			}
			callEquipped(target);
		}

		stack->addSize(-toAdd);
	}
	return stack;
}

ItemStack* PlayerInventory::swap(ItemStack* stack, int index)
{
	
	auto out = takeFromIndex(index, -1);
	putAtIndex(stack, index, -1);
	if(index==trashSlot())//dispose of item
	{
		if(out)
			out->destroy();
		return nullptr;
	}
	return out;

	/*auto t = m_items[index];
	if (index == m_special_hand_slot)
	{
		if (t)
			t->getItem().onUnequipped(*Stats::world, *t, *m_player);
		if(stack)
			stack->getItem().onEquipped(*Stats::world, *stack, *m_player);
	}
	m_items[index] = stack;
	return t;*/
}

ItemStack* PlayerInventory::getItemStack(int index)
{
	return m_items[index];
}

const std::string& PlayerInventory::getID() const
{
	static std::string id = "player_inventory";
	return id;
}

int PlayerInventory::getItemsSize() const
{
	return m_items.size();
}

ItemStack* PlayerInventory::takeFromIndex(int index, int number)
{
	auto& target = m_items[index];
	if (target == nullptr)
		return nullptr;
	if (target->size() <= number || number == -1)
	{
		auto out = target;
		if (index == (int)InventorySlot::HAND)
		{
			callUnequipped(out);
			if (m_special_hand_slot != -1)
				callEquipped(m_items[m_special_hand_slot]);
		}
		else if (index == m_special_hand_slot)
		{
			if (m_special_hand_slot != -1)
				callUnequipped(m_items[m_special_hand_slot]);
		}
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

ItemStack*& PlayerInventory::itemInHand()
{
	if (m_items[InventorySlot::HAND] != nullptr)
		return m_items[InventorySlot::HAND];

	return m_items[m_special_hand_slot == -1 ? InventorySlot::HAND : m_special_hand_slot];
}

int PlayerInventory::itemInHandSlot() const
{
	if (m_items[InventorySlot::HAND] != nullptr)
		return InventorySlot::HAND;

	return m_special_hand_slot == -1 ? InventorySlot::HAND : m_special_hand_slot;
}

ItemStack*& PlayerInventory::handSlot()
{
	return m_items[InventorySlot::HAND];
}

bool PlayerInventory::isSpaceFor(const ItemStack* stack) const
{
	//ignore last slot which is trash
	for (int i = InventorySlot::ACTION_FIRST; i < m_items.size()-1; ++i)
	{
		if (m_items[i] == nullptr)
			return true;
		if (m_items[i]->equals(stack) && !m_items[i]->isFullStack())
			return true;
	}
	return false;
}

void PlayerInventory::setHandIndex(int index)
{
	if (itemInHandSlot() == m_special_hand_slot)
	{
		{
			auto& inHand = itemInHand();
			callUnequipped(inHand);
			m_special_hand_slot = index;
		}
		{
			auto& inHand = itemInHand();
			callEquipped(inHand);
		}
	}
	m_special_hand_slot = index;
}

int PlayerInventory::getHandIndex()
{
	return m_special_hand_slot;
}

void PlayerInventory::save(NBT& src)
{
	src.save("size", m_items.size());
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

void PlayerInventory::load(NBT& src)
{
	m_items.clear();
	NBT& list = src["slots"];
	m_items.resize(src["size"]);

	ZeroMemory(m_items.data(), m_items.size() * sizeof(ItemStack*));
	for (auto value : list.arrays())
		m_items[value["slotter"]] = ItemStack::deserialize(value);
}