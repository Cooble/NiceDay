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
		vecto = entityPlayer->getPosition() - entity->getPosition();

	}else
	{
		vecto = (m_target - entity->getPosition());
	}
	

	if (vecto.lengthSquared() < std::pow(0.5f,2)) {
		m_running = false;
		return TaskResponse::SUCCESS;
	}
	if(entity->isOnFloor()&&(entity->getBlockageState()== PhysEntity::RIGHT||entity->getBlockageState()==PhysEntity::LEFT))
	{
		if (m_last_jump.distanceSquared(entity->getPosition())>0.1f) {
			entity->getVelocity().y = 100;//jmp
			m_last_jump = entity->getPosition();
		}
	}
	entity->getAcceleration().x = abs(vecto.x)/vecto.x * 20.f;
	entity->getAcceleration().y = -9.0f / 60;
	return TaskResponse::RUNNING;
	
	

}
