#include "Sandbox.h"
#include "layer/MainLayer.h"
#include "layer/WorldLayer.h"
#include "graphics/Sprite2D.h"
#include "layer/GUILayer.h"
#include "event/SandboxControls.h"
#include "layer/PriorGenLayer.h"

#ifdef ND_DEBUG
constexpr char* title = "Niceday - Debug";
#else
constexpr char* title = "Niceday - Release";
#endif


Sandbox::Sandbox():
App()
{
	AppInfo info;
	info.title = title;
	info.io.enableSCENE = true;
	info.io.enableMONO = true;
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
