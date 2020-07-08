#include "Sandbox.h"
#include "layer/MainLayer.h"
#include "layer/WorldLayer.h"
#include "graphics/Sprite2D.h"
#include "layer/GUILayer.h"
#include "event/SandboxControls.h"
#include "layer/PriorGenLayer.h"

#ifdef ND_DEBUG
static std::string s_title = "Niceday - Debug";
#else
static std::string s_title = "Niceday - Release";
#endif

Sandbox::Sandbox():
App()
{
	AppInfo info;
	info.title = s_title;
	info.enableSCENE = false;
	init(info);
	Sprite2D::init();
	Controls::init();
	m_LayerStack.pushLayer(new MainLayer());

	m_guiLayer = new GUILayer();
	auto worudo = new WorldLayer();
	m_guiLayer->setWorldLayer(worudo);
	m_LayerStack.pushLayer(worudo);
	m_LayerStack.pushOverlay(m_guiLayer);

	//m_LayerStack.pushLayer(new PriorGenLayer());
}
