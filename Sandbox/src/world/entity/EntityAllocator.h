#pragma once
#include "WorldEntity.h"

class EntityAllocator
{
	
public:

	/***
	 * All entity allocations have to made using this
	 * It will determine whether custom allocation is neccessary
	 */
	static WorldEntity* allocate(EntityType id);

	/***
	 * allocates and constructs entity on returned pointer
	 */
	static WorldEntity* createEntity(EntityType id);

	static WorldEntity* loadInstance(NBT& nbt);
	/***
	 * Every entity needs to be destroyed using this method
	 */
	static void deallocate(WorldEntity* entity);
};
