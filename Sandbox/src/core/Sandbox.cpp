#include "Sandbox.h"
#include "layer/MainLayer.h"
#include "layer/WorldLayer.h"
#include "graphics/Sprite2D.h"

#ifdef ND_DEBUG
static std::string s_title = "Niceday - Debug";
#else
static std::string s_title = "Niceday - Release";
#endif

Sandbox::Sandbox():
App(1280,720, s_title)
{
	Sprite2D::init();
	m_LayerStack.PushLayer(new MainLayer());

	m_LayerStack.PushLayer(new WorldLayer());

	//m_LayerStack.PushLayer(new PriorGenLayer());
}
