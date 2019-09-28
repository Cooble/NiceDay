#include "ndpch.h"
#include "PathTracer.h"
#include "entities.h"


void PathTracer::init(World* world, EntityID physEntity,Phys::Vect acceleration,Phys::Vect maxVelocity)
{
	m_world = world;
	m_phys_entity = physEntity;
	m_acceleration = acceleration;
	m_max_velocity = maxVelocity;
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
	else if(entity->isOnFloor())
		m_last_jump = Phys::Vect(std::numeric_limits<float>::max());
	if(abs(entity->getVelocity().x)<=m_max_velocity.x)
		entity->getAcceleration().x = abs(vecto.x)/vecto.x * m_acceleration.x;
	else entity->getAcceleration().x = 0;

	if (abs(entity->getVelocity().y) <= m_max_velocity.y)
		entity->getAcceleration().y = m_acceleration.y;
	else entity->getAcceleration().y = 0;

	return TaskResponse::RUNNING;
	
	

}
