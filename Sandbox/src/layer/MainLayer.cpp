#include "ndpch.h"
#include "MainLayer.h"
#include "core/Core.h"
#include "event/KeyEvent.h"
#include "core/App.h"
#include "GLFW/glfw3.h"
using namespace nd;

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
		if (event->getKey() == KeyCode::F11) {
			APwin()->setFullScreen(!APwin()->isFullscreen());
			e.handled = true;
			ND_INFO("Fullscreen mode: {}", !APwin()->isFullscreen());
		}
	}
	//ND_INFO("eventos here {0}",e.toString());

}
