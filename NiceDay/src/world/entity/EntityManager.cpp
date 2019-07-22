#include "ndpch.h"
#include "EntityManager.h"

EntityID EntityManager::createEntity()
{
	uint32_t idx;
	if (m_freeList.size() > EE_ENTITY_MINIMAL_FREE_SIZE || (!m_freeList.empty() && m_generations.size() >=
		EE_ENTITY_MAXIMAL_SIZE))
	{
		idx = m_freeList.front();
		m_freeList.pop_front();
	}
	else
	{
		m_loaded.push_back(true);
		m_generations.push_back(0);
		m_p_entities.push_back(nullptr);
		ASSERT(m_generations.size() <= EE_ENTITY_MAXIMAL_SIZE, "Too many entities");
		idx = m_generations.size() - 1;
	}
	m_loaded.set(idx, true);
	return make_entity(idx, m_generations[idx]);
}

void EntityManager::killEntity(EntityID e)
{
	auto idx = index(e);
	if (m_generations[idx] == generation(e))
	{
		++m_generations[idx];
		ASSERT(m_generations[idx] != 0, "We have used one entity slot for MAX_GENERATION times");
		m_freeList.push_back(idx);
	}
}
