#include "ndpch.h"
#include "componentManager.h"

/*
PhysicsComponentManager::PhysicsComponentManager(size_t maxSize)
	: m_max_size(maxSize), m_current_size(0), m_data(maxSize)
{
}

PhysicsInstance PhysicsComponentManager::getInstance(Entity e)
{
	auto f = m_map.find(e);
	if (f != m_map.end())
		return f->second;
	else if(m_current_size<m_max_size)
	{
		m_map[e] = m_current_size;
		m_data.entity[m_current_size] = e;
		return m_current_size++;
	}
	else
	{
		resize(m_max_size + m_max_size / 2);
		return getInstance(e);//this not work hehe
	}

}

void PhysicsComponentManager::removeInstance(Entity e)
{
	PhysicsInstance toremove = getInstance(e);

	--m_current_size;
	PhysicsInstance last = (uint32_t)m_current_size;

	m_map.erase(m_map.find(e));

	m_data.entity[toremove]			= m_data.entity[last];
	m_data.pos[toremove]			= m_data.pos[last];
	m_data.velocity[toremove]		= m_data.velocity[last];
	m_data.acceleration[toremove]	= m_data.acceleration[last];

	m_map[m_data.entity[toremove]] = toremove;
	
}


void PhysicsComponentManager::resize(size_t newSize)
{
	ASSERT(m_max_size < newSize,"cannot shrink");
	m_data.~PhysicsComponentStruct();
	m_data = PhysicsComponentStruct(m_data,newSize);
	m_max_size = newSize;
}

MeshComponentManager::MeshComponentManager(size_t maxSize)
	: m_max_size(maxSize), m_current_size(0), m_data(maxSize)
{
}

MeshInstance MeshComponentManager::getInstance(Entity e)
{
	auto f = m_map.find(e);
	if (f != m_map.end())
		return f->second;
	else if (m_current_size < m_max_size)
	{
		m_map[e] = m_current_size;
		m_data.entity[m_current_size] = e;
		return m_current_size++;
	}
	else
	{
		resize(m_max_size + m_max_size / 2);
	}
	ASSERT(false, "Too small component array");
	return 0;
}

void MeshComponentManager::removeInstance(Entity e)
{
	MeshInstance toremove = getInstance(e);

	--m_current_size;
	MeshInstance last = (uint32_t)m_current_size;

	m_map.erase(m_map.find(e));

	m_data.entity[toremove] = m_data.entity[last];
	m_data.sprite[toremove] = m_data.sprite[last];

	m_map[m_data.entity[toremove]] = toremove;

}


void MeshComponentManager::resize(size_t newSize)
{
	ASSERT(m_max_size < newSize, "cannot shrink");
	m_data.~MeshComponentStruct();
	m_data = MeshComponentStruct(m_data, newSize);
	m_max_size = newSize;
}
*/