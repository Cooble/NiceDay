#pragma once
#include <utility>
#include "ndpch.h"
#include "world/entity/WorldEntity.h"

#define ND_REGISTER_ENTITY(entityType,className)\
	EntityRegistry::get().registerEntity(entityType,className::factoryMethod,sizeof(className),#className)
#define ND_FACTORY_METH_ENTITY_BUILD(className)\
	static WorldEntity* factoryMethod(void* pointer){return new(pointer) className();}

typedef WorldEntity*(*FactoryMethod)(void*);

class EntityRegistry
{
public:
	struct EntityBucket
	{
		EntityType entity_type;
		FactoryMethod method;
		std::string name;
		size_t byte_size;

		EntityBucket(EntityType type, FactoryMethod method, size_t size, std::string name)
			: entity_type(type),
			  method(method),
			  name(std::move(name)),
			  byte_size(size)
		{
		}
	};

private:
	std::vector<EntityBucket> m_entity_buckets;

public:

	void registerEntity(EntityType type, FactoryMethod method, size_t byteSize, std::string name);

	WorldEntity* createInstance(EntityType type, void* pointer) const;

	const std::string& entityTypeToString(EntityType type) const;

	static inline EntityRegistry& get()
	{
		static EntityRegistry s_instance;
		return s_instance;
	}


	const EntityBucket& getBucket(EntityType type) const;
	inline const std::vector<EntityBucket>& getData() const { return m_entity_buckets; }
};
