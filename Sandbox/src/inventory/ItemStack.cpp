#include "ItemStack.h"

#include "core/AppGlobals.h"


using namespace nd;
ItemStack::ItemStack(ItemID item, int size)
	: m_item(item), m_size(size)
{
}

ItemStack::ItemStack(const ItemStack& s)
	: m_item(s.m_item), m_size(s.m_size),m_metadata(s.m_metadata)
{
	if (getItem().hasNBT())
		m_nbt = s.m_nbt;//copy nbt
}

ItemStack::~ItemStack()
{
}

bool ItemStack::equals(const ItemStack* stack) const
{
	if (stack == nullptr)
		return false;
	return *this == *stack;
}

void ItemStack::serialize(NBT& nbt)
{
	nbt.save("size", m_size);
	nbt.save("meta", m_metadata);
	nbt.save("item", m_item);
	if(m_nbt)
		nbt.save("nbt", m_nbt);
}

bool ItemStack::isFullStack() const
{
	return m_size >= getItem().getMaxStackSize();
}

ItemStack* ItemStack::deserialize(const NBT& nbt)
{
	auto out = ItemStack::create(nbt["item"], nbt["size"]);
	out->setMetadata(nbt["meta"]);
	if (nbt.exists("nbt"))
		out->m_nbt = nbt["nbt"];
	return out;
}

bool operator==(const ItemStack& a, const ItemStack& b)
{
	
	return a.getItemID() == b.getItemID() && a.getMetadata() == b.getMetadata();
}


nd::Pool<ItemStack>& ItemStack::s_stack_pool()
{
	static nd::Pool<ItemStack> pool(ITEMSTACK_POOL_SIZE);
	return pool;
}

ItemStack* ItemStack::create(ItemID id, int count)
{
	nd::AppGlobals::get().nbt["item_stack_pool_size"] = s_stack_pool().getCurrentSize();
	return s_stack_pool().allocate(id, count);
}

ItemStack* ItemStack::create(const ItemStack* itemstack)
{
	nd::AppGlobals::get().nbt["item_stack_pool_size"] = s_stack_pool().getCurrentSize();
	return s_stack_pool().allocate(*itemstack);
}



void ItemStack::destroy(ItemStack* stack)
{
	
	s_stack_pool().deallocate(stack);
	nd::AppGlobals::get().nbt["item_stack_pool_size"] = s_stack_pool().getCurrentSize();

}

