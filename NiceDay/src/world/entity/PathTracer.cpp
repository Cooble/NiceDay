#include "ndpch.h"
#include "PathTracer.h"
#include "entities.h"


void PathTracer::init(World* world, EntityID physEntity)
{
	this->m_world = world;
	this->m_phys_entity = physEntity;

}

TaskResponse PathTracer::update()
{
	//get src entity ID
	auto entityP = m_world->getLoadedEntity(m_phys_entity);
	if (!entityP) {
		m_running = false;
		return TaskResponse::FAIL;
	}
	auto entity = dynamic_cast<PhysEntity*>(entityP);
	if (!entity) {
		m_running = false;
		return TaskResponse::FAIL;
	}
	Phys::Vect vecto;
	if(m_entity_target!=ENTITY_ID_INVALID)
	{
		//get target entity ID
		auto entityPlayer = m_world->getLoadedEntity(m_entity_target);
		if (!entityPlayer) {
			m_running = false;
			return TaskResponse::FAIL;
		}
		vecto = (entityPlayer->getPosition() - entity->getPosition());

	}else
	{
		vecto = (m_target - entity->getPosition());
	}

	if (vecto.lengthSquared() < std::pow(0.5f,2)) {
		m_running = false;
		return TaskResponse::SUCCESS;
	}
	vecto.normalize();
	entity->getVelocity() = vecto * 10.f;

	return TaskResponse::RUNNING;
	
	

}
