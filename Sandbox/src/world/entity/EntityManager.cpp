#include "ndpch.h"
#include "EntityManager.h"

EntityID EntityManager::createEntity()
{
	/*std::vector<bool> bools;
	for (int i = 0; i < m_loaded.bitSize(); ++i)
		bools.push_back(m_loaded[i]);*/
	
	uint32_t idx;
	if (m_freeList.size() > EE_ENTITY_MINIMAL_FREE_SIZE || (!m_freeList.empty() && m_generations.size() >=
		EE_ENTITY_MAXIMAL_SIZE))
	{
		idx = m_freeList.front();
		m_freeList.pop_front();
	}
	else
	{
		m_loaded.push_back(false);
		m_generations.push_back(0);
		m_p_entities.push_back(nullptr);
		ASSERT(m_generations.size() <= EE_ENTITY_MAXIMAL_SIZE, "Too many entities");
		idx = m_generations.size() - 1;
	}
	m_loaded.resize(idx + 1);
	m_loaded.set(idx, false);
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


struct MHeader{
	uint32_t freeListSize;
	uint32_t generationSize;
};

void EntityManager::serialize(IStream* stream)
{
	MHeader h = {};
	h.freeListSize = m_freeList.size();
	h.generationSize = m_generations.size();
	stream->write(h);
	for (uint32_t value : m_freeList)
		stream->write(value);

	stream->write((char*)m_generations.data(), h.generationSize * sizeof(uint8_t));
}

void EntityManager::deserialize(IStream* stream)
{
	MHeader h={};
	stream->read(h);
	uint32_t fre = 0;
	for (int i = 0; i < h.freeListSize; ++i)
	{
		stream->read(fre);
		m_freeList.push_back(fre);
	}
	m_generations.resize(h.generationSize);
	stream->read((char*)m_generations.data(), h.generationSize * sizeof(uint8_t));

	m_loaded.resize(h.generationSize);
	m_p_entities.resize(h.generationSize);
}
