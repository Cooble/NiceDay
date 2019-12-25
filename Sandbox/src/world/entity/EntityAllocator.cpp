#include "EntityAllocator.h"
#include "EntityRegistry.h"

WorldEntity* EntityAllocator::allocate(EntityType id)
{
	auto& bucket = EntityRegistry::get().getBucket(id);
	auto entity = (WorldEntity*)malloc(bucket.byte_size);
	return entity;
	
}

WorldEntity* EntityAllocator::createEntity(EntityType id)
{
	auto pointer = allocate(id);
	return EntityRegistry::get().createInstance(id, pointer);
}

WorldEntity* EntityAllocator::loadInstance(NBT& nbt)
{
	ASSERT(nbt.exists<EntityType>("entityTypeID"), "Invalid entity nbt");
	EntityType type = nbt.get<EntityType>("entityTypeID");
	auto entity = createEntity(type);
	entity->load(nbt);
	return entity;
}

void EntityAllocator::deallocate(WorldEntity* entity)
{
	free(entity);
}

