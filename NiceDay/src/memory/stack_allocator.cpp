#include "ndpch.h"
#include "memory/stack_allocator.h"
#include "core/App.h"

namespace nd::internal {

void* allocateMeeh(size_t n)
{
	return App::get().getBufferedAllocator().allocate(n);
}

void* allocateMeehStandard(size_t n)
{
	if (App::get().getMainThreadID() == std::this_thread::get_id())
		return App::get().getBufferedAllocator().allocate(n);

	return malloc(n);
}

void deallocateMeehStandard(void* p)
{
	if (App::get().getMainThreadID() == std::this_thread::get_id())
	{
	}
	else
		free(p);
}
}
