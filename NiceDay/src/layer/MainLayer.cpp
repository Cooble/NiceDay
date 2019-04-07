#include "ndpch.h"
#include "MainLayer.h"
#include "Core.h"

MainLayer::MainLayer()
	:Layer("MainLayer"){}

MainLayer::~MainLayer(){
}

void MainLayer::onAttach()
{
	//ND_INFO("Attached layer");

}
void MainLayer::onDetach()
{
	//ND_INFO("deatched layer");

}
void MainLayer::onUpdate()
{
	//ND_INFO("update layer");

}
void MainLayer::onImGuiRender()
{

}
void MainLayer::onEvent(Event& e)
{
	//ND_INFO("eventos here {0}",e.toString());

}
