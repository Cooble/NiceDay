﻿#include "ndpch.h"
#include "EntityRegistry.h"


void EntityRegistry::registerEntity(EntityType type, FactoryMethod method, size_t byteSize, std::string name)
{
	for (auto& entity_bucket : m_entity_templates)
	{
		if (entity_bucket.entity_type == type)
			return;
	}
	m_entity_templates.emplace_back(type, method, byteSize,name);
}

WorldEntity* EntityRegistry::createInstance(EntityType type, void* pointer) const
{
	for (auto& entity_bucket : m_entity_templates)
	{
		if (entity_bucket.entity_type == type)
			return entity_bucket.factory_method(pointer);
	}
	ASSERT(false, "Invalid EntityType");
	return nullptr;
}

const std::string& EntityRegistry::entityTypeToString(EntityType type) const
{
	for (auto& entity_bucket : m_entity_templates)
	{
		if (entity_bucket.entity_type == type)
			return entity_bucket.name;
	}
	ASSERT(false, "Invalid EntityType");
	static std::string s = "invalidEntity";
	return s;
}

const EntityRegistry::EntityTemplate& EntityRegistry::getBucket(EntityType type) const
{
	for (auto& entity_bucket : m_entity_templates)
	{
		if (entity_bucket.entity_type == type)
			return entity_bucket;
	}
	ASSERT(false, "Invalid EntityType");
}

EntityType EntityRegistry::getEntityType(const std::string& name)
{
	for (auto& entity_bucket : m_entity_templates)
	{
		if (entity_bucket.name == name)
			return entity_bucket.entity_type;
	}
	return ENTITY_ID_INVALID;
}
