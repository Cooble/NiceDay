#include "ndpch.h"
#include "NBT.h"

Pool<NBT>& NBT::s_stack_pool()
{
	static Pool<NBT> pool(1000);
	return pool;
}

NBT* NBT::create()
{
	return s_stack_pool().allocate();
}

NBT* NBT::create(const NBT& nbt)
{
	return s_stack_pool().allocate(nbt);
}

void NBT::destroy(NBT* stack)
{
	return s_stack_pool().deallocate(stack);
}

NBT* NBT::copy() const
{
	return NBT::create(*this);
}

void NBT::destroy()
{
	NBT::destroy(this);
}
