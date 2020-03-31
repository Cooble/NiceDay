#pragma once
#include "ndpch.h"
#include "core/NBT.h"


class AppGlobals
{
public:
	static AppGlobals& get();
public:
	NBT nbt;
	
};
#define ND_GLOBAL_LOG(name,val) AppGlobals::get().nbt.save((name),(val))
