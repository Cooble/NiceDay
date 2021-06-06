#pragma once

#define EE_ENTITY_MINIMAL_FREE_SIZE 255
#define EE_ENTITY_MAXIMAL_SIZE (1<<23)
#include "core/IBinaryStream.h"

class WorldEntity;

typedef uint32_t EntityID;
typedef uint32_t EntityType;

//todo in code the usage is misused with entityID when it should be entity type instead
constexpr EntityID ENTITY_ID_INVALID = std::numeric_limits<EntityID>::max() >> 2;

class EntityManager
{
private:
	std::deque<uint32_t> m_freeList;
	std::vector<uint8_t> m_generations;
	std::vector<WorldEntity*> m_p_entities;
	NDUtils::Bitset m_loaded;
private:
	inline uint32_t index(EntityID e) { return e >> 8; }
	inline uint32_t generation(EntityID e) { return (uint8_t)e; }
	inline EntityID make_entity(uint32_t index, uint8_t gen) { return index << 8 | gen; }

public:
	// returns new id for entity which:
	//			- is alive
	//			- has nullptr 
	//			- is not loaded
	EntityID createEntity();

	void killEntity(EntityID e);

	inline bool isAlive(EntityID e)
	{
		auto idx = index(e);
		return idx < m_p_entities.size() && m_generations[idx] == generation(e);
	}

	inline bool isLoaded(EntityID e)
	{
		auto idx = index(e);
		return idx < m_p_entities.size() && m_loaded[index(e)];
	}

	inline void setLoaded(EntityID e, bool loaded)
	{
		auto idx = index(e);
		if (idx >= m_p_entities.size())
		{
			ASSERT(false, "invalid entityID");
			return;
		}
		m_loaded.set(idx, loaded);
	}

	inline void setEntityPointer(EntityID id, WorldEntity* pointer)
	{
		auto idx = index(id);
		if (idx >= m_p_entities.size())
		{
			ASSERT(false, "invalid entityID");
			return;
		}
		m_p_entities[idx] = pointer;
	}

	inline WorldEntity* entity(EntityID e)
	{
		auto idx = index(e);
		if (idx >= m_p_entities.size())
			return nullptr;
		//this complete utter mess is used instead of if statement to avoid branch misprediction, yay we have saved maybe 5ns :D (and maybe it is even slower) i dont want to check it
		return (WorldEntity*)((size_t)m_p_entities[idx] * (m_loaded[idx] && m_generations[idx] == generation(e)));
		//check if entity is loaded (otherwise it would output pointer to invalid location)
	}

	// returns entity pointer no matter if it was unloaded or not (pointer might reference to invalid location!)
	// use only if youre sure that entity is loaded (and alive so to speak)
	inline WorldEntity* entityDangerous(EntityID e) { return m_p_entities[index(e)]; }

	void serialize(const nd::IBinaryStream::RWStream& stream);
	void deserialize(const nd::IBinaryStream::RWStream& stream);
};
