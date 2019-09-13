#include "ndpch.h"
#include "MainLayer.h"
#include "Core.h"
#include "event/KeyEvent.h"
#include "App.h"

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
	if(e.getEventType()==Event::EventType::KeyPress)
	{
		auto event = dynamic_cast<KeyPressEvent*>(&e);
		if (event->getKey() == GLFW_KEY_F11) {
			App::get().getWindow()->setFullScreen(!App::get().getWindow()->isFullscreen());
			e.handled = true;
			ND_INFO("Fullscreen mode: {}", !App::get().getWindow()->isFullscreen());
		}
	}
	//ND_INFO("eventos here {0}",e.toString());

}
