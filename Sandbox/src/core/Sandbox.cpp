#include "Sandbox.h"

#include "Translator.h"
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

	//language setup
	AppLanguages::registerLanguage("English", "en");
	AppLanguages::registerLanguage("Cestina", "cs");
	AppLanguages::addLanguageFolder(ND_RESLOC("res/lang"));
	std::string l;
	getSettings().load("language", l, "en");
	AppLanguages::loadLanguage(l);
	
	m_guiLayer = new GUILayer();
	auto worudo = new WorldLayer();
	m_guiLayer->setWorldLayer(worudo);
	m_LayerStack.pushLayer(worudo);
	m_LayerStack.pushOverlay(m_guiLayer);

	//m_LayerStack.pushLayer(new PriorGenLayer());
}

