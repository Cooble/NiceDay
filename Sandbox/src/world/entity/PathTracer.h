#pragma once
#include "world/World.h"
#include "EntityManager.h"

enum class TaskResponse
{
	FAIL,
	SUCCESS,
	RUNNING
};
class PathTracer
{
private:
	World* m_world;
	EntityID m_phys_entity;
	Phys::Vect m_target;
	EntityID m_entity_target=ENTITY_ID_INVALID;
	Phys::Vect m_acceleration;
	Phys::Vect m_max_velocity;

	Phys::Vect m_last_jump;
	bool m_running=false;
public:

	PathTracer() = default;
	void init(World* world, EntityID physEntity, Phys::Vect acceleration, Phys::Vect maxVelocity);

	inline void setTarget(const Phys::Vect& target)
	{
		if(m_target!=target)//new order new rules
			m_running = true;
		m_target = target;
		m_entity_target = ENTITY_ID_INVALID;
	}
	inline void setTarget(EntityID targetID)
	{
		if (m_entity_target!= targetID)//new order new rules
			m_running = true;
		m_entity_target = targetID;
	}


	inline bool isRunning() const { return m_running; }

	TaskResponse update();
	
};
