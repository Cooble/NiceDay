#pragma once
#include <utility>
#include "ndpch.h"
#include "world/entity/WorldEntity.h"

#define ND_REGISTER_ENTITY(entityType,className)\
	EntityRegistry::get().registerEntity(entityType,className::factoryMethod,sizeof(className),#className)
#define ND_FACTORY_METH_ENTITY_BUILD(className)\
	static WorldEntity* factoryMethod(void* pointer){return new(pointer) className();}

typedef WorldEntity*(*FactoryMethod)(void*);
//typedef void (*InitMethod)(void*);

class EntityRegistry
{
public:
	struct EntityTemplate
	{
		EntityType entity_type;
		FactoryMethod factory_method;
		//InitMethod init_method;
		std::string name;
		size_t byte_size;

		EntityTemplate(EntityType type, FactoryMethod method, size_t size, std::string name)
			: entity_type(type),
			  factory_method(method),
			//  init_method(initm),
			  name(std::move(name)),
			  byte_size(size)
		{
		}
	};

private:
	std::vector<EntityTemplate> m_entity_templates;

public:

	void registerEntity(EntityType type, FactoryMethod method, size_t byteSize, std::string name);

	WorldEntity* createInstance(EntityType type, void* pointer) const;

	const std::string& entityTypeToString(EntityType type) const;
	

	static EntityRegistry& get()
	{
		static EntityRegistry s_instance;
		return s_instance;
	}


	const EntityTemplate& getBucket(EntityType type) const;
	EntityType getEntityType(const std::string& name);
	const std::vector<EntityTemplate>& getData() const { return m_entity_templates; }
};
