#include "PlayerInventory.h"
#include "world/World.h"
#include "core/Stats.h"
#include "core/NBT.h"
#include <stack>

using namespace nd;

PlayerInventory::~PlayerInventory()
{
	for (auto item : m_items)
		if (item)item->destroy();
}

PlayerInventory::PlayerInventory(WorldEntity* player)
	: m_player(player)
{
	m_special_hand_slot = -1;
	constexpr int inventorySize = ACTION_FIRST + 10 + 40;
	//9 activeslots + 27 random items

	m_items.resize(inventorySize);
	ZeroMemory(m_items.data(), inventorySize * sizeof(ItemStack*));
}

void PlayerInventory::callEquipped(ItemStack* itemStack)
{
	if (itemStack) {
		m_item_data_box = itemStack->getItem().instantiateDataBox();
		itemStack->getItem().onEquipped(*Stats::world, *itemStack, *m_player);
	}
}

void PlayerInventory::callUnequipped(ItemStack* itemStack)
{
	if (itemStack) {
		itemStack->getItem().onUnequipped(*Stats::world, *itemStack, *m_player);
		if (m_item_data_box)
			itemStack->getItem().destroyDataBox(m_item_data_box);
		m_item_data_box = nullptr;
	}
}

bool PlayerInventory::canPutAtIndex(ItemStack* itemStack, int index)
{
	// we can always throw away trash or nullptr
	if (index == trashSlot() || !itemStack)
		return true;
	auto& item = itemStack->getItem();
	if (index == ARMOR_HELMET && !item.hasFlag(ITEM_FLAG_ARMOR_HEAD))
		return false;
	if (index == ARMOR_CHEST && !item.hasFlag(ITEM_FLAG_ARMOR_CHEST))
		return false;
	if (index == ARMOR_LEGGINS && !item.hasFlag(ITEM_FLAG_ARMOR_LEGGINS))
		return false;
	if (index == ARMOR_BOOTS && !item.hasFlag(ITEM_FLAG_ARMOR_BOOTS))
		return false;
	
	// free slot
	if(!m_items[index])
		return true;
	
	// same item and there is still some space 
	return m_items[index]->equals(itemStack) && item.getMaxStackSize() != m_items[index]->size();
}

ItemStack* PlayerInventory::putAtRandomIndex(ItemStack* stack)
{
	auto& item = stack->getItem();

	int toAdd = stack->size();
	int lastNullFreeIndex = -1;
	bool isStackable = item.getMaxStackSize() > 1;
	for (int i = ACTION_FIRST; i < m_items.size() - 1; ++i)//dont put to trash
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
	if (!stack)
		return nullptr;
	
	if (!canPutAtIndex(stack, index))
		return stack;

	auto& target = m_items[index];

	if (index == trashSlot())// destroy thing currently in trash and add stack to trash slot
	{
		// todo throwing in trash ignores count and expects count to be -1 (everything)
		// todo add partial destroy maybe
		if (target != nullptr)
			target->destroy();
		target = stack;
		return nullptr;
	}

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
		int toAdd = count == ALL ? stack->size() : count;
		if (toAdd == stack->size())
		{
			target = stack;
			if (index == m_special_hand_slot)
				callEquipped(stack);
			else if (index == (int)HAND)
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
		else if (index == (int)HAND)
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
	if(index==trashSlot())
	{
		if(m_items[index])
			m_items[index]->destroy();
		m_items[index] = stack;
		return nullptr;
	}
	auto taken = takeFromIndex(index, -1);
	auto remains = putAtIndex(stack, index, -1);

	if (remains != nullptr) // something is still at that slot
	{
		// put back what we have taken
		auto e = putAtIndex(taken, index, -1);
		if (e) {
			ND_WARN("cannot put item back to slot during unsuccessful item swap!");
			e->destroy(); // we have no other option but to delete the item
		}
		return remains;
	}
	return taken;
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
	if (target->size() <= number || number == ALL)
	{
		auto out = target;
		if (index == (int)HAND)
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
	if (m_items[HAND] != nullptr)
		return m_items[HAND];

	return m_items[m_special_hand_slot == -1 ? HAND : m_special_hand_slot];
}

int PlayerInventory::itemInHandSlot() const
{
	if (m_items[HAND] != nullptr)
		return HAND;

	return m_special_hand_slot == -1 ? HAND : m_special_hand_slot;
}

ItemStack*& PlayerInventory::handSlot()
{
	return m_items[HAND];
}

bool PlayerInventory::isSpaceFor(const ItemStack* stack) const
{
	//ignore last slot which is trash
	for (int i = ACTION_FIRST; i < m_items.size() - 1; ++i)
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