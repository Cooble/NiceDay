#include "ndpch.h"
#include "WorldLayer.h"
#include "world/WorldGen.h"
#include "Core.h"

WorldLayer::WorldLayer()
	:Layer("WorldLayer") 
{
	WorldGenInfo info;
	info.world_name = "NiceWorld";
	info.chunk_width = 20;
	info.chunk_height = 10;
	info.seed = 0;
	m_world = WorldGen::genWorld(info);

	


}

WorldLayer::~WorldLayer() {
	delete m_world;
}

void WorldLayer::onAttach()
{

}
void WorldLayer::onDetach()
{

}
void WorldLayer::onUpdate()
{
	m_world->onUpdate();
}
void WorldLayer::onRender()
{
	m_world->onRender();
}
void WorldLayer::onImGuiRender()
{

}
void WorldLayer::onEvent(Event& e)
{

}
