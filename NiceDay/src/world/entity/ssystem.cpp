#include "ndpch.h"
#include "ssystem.h"
#include "graphics/BatchRenderer2D.h"
#include "world/World.h"
/*
MeshSystem::MeshSystem(BatchRenderer2D& r)
	: m_renderer(r)
{
}

void MeshSystem::proccess(MeshComponentManager& mesh, PhysicsComponentManager& point)
{
	auto size = mesh.getCurrentSize();
	for (int i = 0; i < size; ++i)
	{
		auto& t = mesh.mesh(i);
		Entity e = mesh.entity(i);
		auto& v = point.position(point.getInstance(e));
		t.setPosition(vec3(v.x, v.y, 0));
		t.render(m_renderer);
	}
}

PhysicsSystem::PhysicsSystem(World& w)
	: m_world(w)
{
}

void PhysicsSystem::proccess(PhysicsComponentManager& point)
{
	auto size = point.getCurrentSize();
	vec2 pT;
	vec2 vT;
	for (int i = 0; i < size; ++i)
	{
		auto& p = point.position(i);
		auto& a = point.acceleration(i);
		auto& v = point.velocity(i);

		pT = point.position(i);
		vT = point.velocity(i);

		vT += a;
		pT += vT;

		auto& b = m_world.getBlock((int)pT.x, (int)pT.y);

		if (b.isAir()) //we wont hit the block
		{
			v += a;
			p += v;
		}
		else
		{
			v = {0, 0};
		}
	}
}

void PhysicsSystem2::proccess(PhysicsComponentManager& point)
{
	auto size = point.getCurrentSize();
	for (int i = 0; i < size; ++i)
	{
		auto& p = point.position(i);
		auto& a = point.acceleration(i);
		auto& v = point.velocity(i);

		v += a;
		p += v;
	}
}
*/