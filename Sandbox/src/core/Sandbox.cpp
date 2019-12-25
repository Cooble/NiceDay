#include "Sandbox.h"
#include "layer/MainLayer.h"
#include "layer/WorldLayer.h"
#include "graphics/Sprite2D.h"
#include "layer/GUILayer.h"

#ifdef ND_DEBUG
static std::string s_title = "Niceday - Debug";
#else
static std::string s_title = "Niceday - Release";
#endif

Sandbox::Sandbox():
App(1280,720, s_title)
{
	//m_imgui_enable = false;
	Sprite2D::init();
	m_LayerStack.pushLayer(new MainLayer());

	m_guiLayer = new GUILayer();
	auto worudo = new WorldLayer();
	m_guiLayer->setWorldLayer(worudo);
	m_LayerStack.pushLayer(worudo);
	m_LayerStack.pushLayer(m_guiLayer);

	//m_LayerStack.PushLayer(new PriorGenLayer());
}
