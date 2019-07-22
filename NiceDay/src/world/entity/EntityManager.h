#pragma once

#define EE_ENTITY_MINIMAL_FREE_SIZE 255
#define EE_ENTITY_MAXIMAL_SIZE (1<<23)


class WorldEntity;

typedef uint32_t EntityID;
typedef uint32_t EntityType;

constexpr EntityID ENTITY_ID_INVALID=std::numeric_limits<EntityID>::max();

class EntityManager
{
private:
	std::deque<uint32_t> m_freeList;
	std::vector<uint8_t> m_generations;
	std::vector<WorldEntity*> m_p_entities;
	NDUtil::Bitset m_loaded;
private:
	inline uint32_t index(EntityID e) { return e >> 8; }
	inline uint32_t generation(EntityID e) { return ((uint8_t)e); }
	inline EntityID make_entity(uint32_t index, uint8_t gen) { return (index << 8) | gen; }

public:
	EntityID createEntity();

	void killEntity(EntityID e);

	inline bool isAlive(EntityID e) { return m_generations[index(e)] == generation(e); }
	inline bool isLoaded(EntityID e) { return m_loaded[index(e)]; }
	inline void setLoaded(EntityID e, bool loaded) { m_loaded.set(index(e), loaded); }

	inline void setEntityPointer(EntityID id, WorldEntity* pointer) { m_p_entities[index(id)] = pointer; }

	inline WorldEntity* entity(EntityID e)
	{
		auto idx = index(e);
		//this complete utter mess is used instead of if statement to avoid branch misprediction, yay we have saved maybe 5ns :D (and maybe it is even slower) i dont want to check it
		return (WorldEntity*)(((size_t)m_p_entities[idx])*m_loaded[idx]);//check if entity is loaded (otherwise it would output pointer to invalid location)
	}

	// returns entity pointer no matter if it was unloaded or not (pointer might reference to invalid location!)
	// use only if youre sure that entity is loaded (and alive so to speak)
	inline WorldEntity* entityDangerous(EntityID e)
	{
		return m_p_entities[index(e)];
	}
};
