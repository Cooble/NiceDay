#pragma once
#include "ndpch.h"
#include "nbt/NBT.h"


class AppGlobals
{
public:
	static AppGlobals& get();
public:
	NBT nbt;
	
};
#define ND_GLOBAL_LOG(name,val) AppGlobals::get().nbt.set((name),(val))
