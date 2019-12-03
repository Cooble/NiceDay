#pragma once
#include "memory/stack_allocator.h"

enum WindowMess:int{
	MenuPlay,
	MenuPlayWorld,
	MenuDeleteWorld,
	MenuExit,
	MenuSettings,
	MenuGenerateWorld,
	MenuBack
};
namespace WindowMessageData
{
	struct World
	{
		nd::temp_string worldName;
	};
};
