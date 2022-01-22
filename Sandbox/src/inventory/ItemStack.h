#pragma once
#include "ndpch.h"
#include "ItemRegistry.h"
#include "core/NBT.h"
#include "memory/Pool.h"



class ItemStack
{
private:
	static nd::Pool<ItemStack>& s_stack_pool();
public:
	static ItemStack* create(ItemID id, int  count = 1);
	static ItemStack* create(const ItemStack* itemstack);
	static ItemStack* deserialize(const nd::NBT& nbt);
	static void destroy(ItemStack* stack);
private:
	ItemID m_item;
	uint64_t m_metadata = 0;
	int m_size;
	nd::NBT m_nbt;
public:
	ItemStack(ItemID item, int size = 1);
	ItemStack(const ItemStack& s);
	~ItemStack();

	uint64_t getMetadata() const { return m_metadata; }
	void setMetadata(uint64_t meta) { m_metadata = meta; }
	void setSize(int size) { m_size = size; }
	int size()const { return m_size; }
	bool isEmpty() const { return m_size == 0; }
	ItemID getItemID() const { return m_item; }
	const Item& getItem() const { return ItemRegistry::get().getItem(m_item); }
	const nd::NBT& getNBT() const { return m_nbt; }
	nd::NBT& getNBT() { return m_nbt; }
	void destroy() { ItemStack::destroy(this); }
	ItemStack* copy() const { return ItemStack::create(this); }
	bool equals(const ItemStack* stack) const;

	void addSize(int count)
	{
		if (m_size == Item::INFINITE_SIZE)//ignore change if item has infinite size
			return;
		ASSERT(m_size + count <= getItem().getMaxStackSize(), "Too big itemstack, ({}/{})", m_size + count, getItem().getMaxStackSize());
		m_size += count;
		if (m_size < 0)
			m_size = 0;
	}
	void serialize(nd::NBT& nbt);
	bool isFullStack() const;
};
bool operator==(const ItemStack& a, const ItemStack& b);
