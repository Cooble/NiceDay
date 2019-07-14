#include "ndpch.h"
#include "EntityRegistry.h"


void EntityRegistry::registerEntity(EntityType type, FactoryMethod method, size_t byteSize, std::string name)
{
	for (auto& entity_bucket : m_entity_buckets)
	{
		if (entity_bucket.entity_type == type)
			return;
	}
	m_entity_buckets.emplace_back(type, method, byteSize,name);
}

WorldEntity* EntityRegistry::createInstance(EntityType type, void* pointer) const
{
	for (auto& entity_bucket : m_entity_buckets)
	{
		if (entity_bucket.entity_type == type)
			return entity_bucket.method(pointer);
	}
	ASSERT(false, "Invalid EntityType");
	return nullptr;
}

const std::string& EntityRegistry::entityTypeToString(EntityType type) const
{
	for (auto& entity_bucket : m_entity_buckets)
	{
		if (entity_bucket.entity_type == type)
			return entity_bucket.name;
	}
	ASSERT(false, "Invalid EntityType");
}

const EntityRegistry::EntityBucket& EntityRegistry::getBucket(EntityType type) const
{
	for (auto& entity_bucket : m_entity_buckets)
	{
		if (entity_bucket.entity_type == type)
			return entity_bucket;
	}
	ASSERT(false, "Invalid EntityType");
}
