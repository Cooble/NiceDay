#include "ndpch.h"
#include "memory/stack_allocator.h"
#include "core/App.h"

void* allocateMeeh(size_t n)
{
	return App::get().getBufferedAllocator().allocate(n);
}
