#include "GUILayer.h"

#include <imguifiledialog/ImGuiFileDialog.h>


#include "imgui.h"
#include "Translator.h"
#include "gui/GUIContext.h"
#include "event/WindowEvent.h"
#include "core/App.h"
#include "gui/MainWindow.h"
#include "event/MessageEvent.h"
#include "WorldLayer.h"
#include "core/AppGlobals.h"
#include "core/ImGuiLayer.h"
#include "gui/window_messeages.h"
#include "event/KeyEvent.h"
#include "GLFW/glfw3.h"
#include "gui/GUIEntityPlayer.h"
#include "world/entity/EntityPlayer.h"
#include "event/SandboxControls.h"
#include "gui/GUIEntityCreativeTab.h"

#include <stack>

using namespace nd;

GUILayer::GUILayer()
{
	ND_PROFILE_METHOD();
	m_bound_func = std::bind(&GUILayer::consumeWindowEvent, this, std::placeholders::_1);
	m_gui_context = GUIContext::create();
	m_gui_renderer.setContext(&GUIContext::get());
	//m_gui_renderer.m_font_material = FontMatLib::getMaterial("res/fonts/andrew_big.fnt");
	std::string lang;
	App::get().getSettings().loadSet("language", lang, "en");
	if (lang == "jp")
	{
		GameFonts::smallFont = FontMatLib::getMaterial("res/fonts/umeboshi.fnt");
		GameFonts::bigFont = FontMatLib::getMaterial("res/fonts/umeboshi_big.fnt");
	}
	else {
		GameFonts::smallFont = FontMatLib::getMaterial("res/fonts/andrew_czech.fnt");
		GameFonts::bigFont = FontMatLib::getMaterial("res/fonts/andrew_big_czech.fnt");
	}
	m_gui_renderer.m_font_material = GameFonts::smallFont;
	m_background = Texture::create(
		TextureInfo("res/images/logos/back_logo.png").wrapMode(TextureWrapMode::CLAMP_TO_EDGE));
}

GUILayer::~GUILayer()
{
	GUIContext::destroy(m_gui_context);
}

void GUILayer::setWorldLayer(WorldLayer* l)
{
	m_world = l;
	m_gui_renderer.setItemAtlas(16,
		Texture::create(
			TextureInfo("res/images/itemAtlas/atlas.png").
			filterMode(TextureFilterMode::NEAREST)));
}

void GUILayer::updateWorldList()
{
	WorldsProvider::get().rescanWorlds();
	((SelectWorldWindow*)m_currentWindow)->setWorlds(WorldsProvider::get().getAvailableWorlds());
}


static std::stack<WindowMess> windows;
void GUILayer::openWindow(WindowMess mess) {

	if (m_currentWindow) {
		GUIContext::get().destroyWindow(m_currentWindow->serialID);
		m_currentWindow = nullptr;
	}

	if (mess != OpenBack && mess != OpenBackToMain)
		windows.push(mess);

	switch (mess) {
	case OpenMain:
		m_currentWindow = new MainWindow(m_bound_func);
		break;
	case OpenWorldSelection:
		m_currentWindow = new SelectWorldWindow(m_bound_func);
		break;
	case OpenPlayWorld:
		m_hud = new HUD();
		m_currentWindow = m_hud;
		break;
	case OpenExit:
		App::get().fireEvent(WindowCloseEvent());
		return;
	case OpenSettings:
		m_currentWindow = new SettingsWindow(m_bound_func);
		break;
	case OpenControls:
		m_currentWindow = new ControlsWindow(m_bound_func);
		break;
	case OpenLanguage:
		m_currentWindow = new LanguageWindow(m_bound_func);
		break;
	case OpenPause:
		m_currentWindow = new PauseWindow(m_bound_func);
		break;
	case OpenSkin:
		m_currentWindow = new SkinWindow(m_bound_func);
		break;
	case OpenBack: {
		windows.pop();//pop current window
		auto mess = windows.top();//get parent window
		windows.pop();//go level up
		openWindow(mess);
		return;
	}
	case OpenBackToMain: {
		while (!windows.empty())
			windows.pop();
		openWindow(OpenMain);
		return;
	}
	default:;
	}
	GUIContext::get().openWindow(m_currentWindow);
}


void GUILayer::proccessWindowEvent(const MessageEvent& e)
{

	auto message = (WindowMess)e.getID();
	if (WindowMessageData::isOpenMessage(message))
		openWindow(message);


	auto worldData = (WindowMessageData::World*)e.getData();
	switch (e.getID())
	{
	case WindowMess::OpenWorldSelection:
		updateWorldList();
		break;
	case WindowMess::OpenPlayWorld:
	{
		m_game_screen = GameScreen::World;
		m_world->loadWorld(worldData->worldName, false);
		m_background_enable = false;
	}
	break;
	case WindowMess::OpenBack:
	{
		if (m_game_screen == GameScreen::Pause)
		{
			m_game_screen = GameScreen::World;
			m_world->pause(false);
		}
	}
	break;
	case WindowMess::OpenPause:
		m_world->pause(true);
		m_game_screen = GameScreen::Pause;
		break;
	case WindowMess::ActionGenerateWorld:
	{
		bool duplicate = false;
		for (auto& data : WorldsProvider::get().getAvailableWorlds())
		{
			if (strcmp(data.name.c_str(), worldData->worldName.c_str()) == 0)
			{
				duplicate = true;
				break;
			}
		}
		if (duplicate)
			break;
		m_game_screen = GameScreen::World;
		m_background_enable = false;
		m_world->loadWorld(worldData->worldName, true);
		openWindow(WindowMess::OpenPlayWorld);
	}
	break;
	case WindowMess::ActionDeleteWorld:
		WorldsProvider::get().deleteWorld(std::string(worldData->worldName));
		updateWorldList();
		break;

	case WindowMess::ActionWorldQuit:
	{
		ND_INFO("World unloading:::");
		m_world->onDetach();
		m_world->onAttach();
		ND_INFO("World unloading:::done");
		m_background_enable = true;
		m_game_screen = GameScreen::GUI;
		openWindow(WindowMess::OpenBackToMain);
	}
	break;
	}
}

void GUILayer::consumeWindowEvent(const MessageEvent& e)
{
	m_window_event_buffer.push_back(e);
}

static bool ImGUIopen = false;
void GUILayer::onAttach()
{
	REGISTER_IMGUI_WIN("GUILayer", &ImGUIopen);
	ND_PROFILE_METHOD();
	m_game_screen = GameScreen::GUI;
	GUIContext::setContext(m_gui_context);
	openWindow(WindowMess::OpenMain);
}

void GUILayer::onDetach()
{
	GUIContext::get().destroyWindows();
}

void GUILayer::onUpdate()
{
	GUIContext::setContext(m_gui_context);
	GUIContext::get().onUpdate();
	for (auto& event : m_window_event_buffer)
		proccessWindowEvent(event);
	m_window_event_buffer.clear();
}

void GUILayer::onRender()
{
	m_renderer.begin(Renderer::getDefaultFBO());
	Gcon.enableDepthTest(true);
	if (m_background_enable)
		m_renderer.submitTextureQuad({ -1, -1.f, 0.9 }, { 2.f, 2 }, UVQuad::elementary(), m_background);
	m_renderer.push(
		glm::translate(
			glm::mat4(1.f),
			{ -1.f, -1.f, 0 }));
	m_renderer.push(
		glm::scale(
			glm::mat4(1.f),
			{ 2.f / APwin()->getWidth(), 2.f / APwin()->getHeight(), 1 }));
	//todo gui renderer does not need depth test possibly?
	m_gui_renderer.render(m_renderer);
	m_renderer.flush();
	m_renderer.pop();
	m_renderer.pop();
}



void GUILayer::onEvent(Event& e)
{
	bool flipped = e.isInCategory(Event::EventCategory::CatMouse);
	if (flipped)
		static_cast<MouseMoveEvent&>(e).flipY(APwin()->getHeight());
	GUIContext::setContext(m_gui_context);
	GUIContext::get().onEvent(e);
	if (flipped)
		static_cast<MouseMoveEvent&>(e).flipY(APwin()->getHeight());
	if (e.handled)
		return;
	if (m_game_screen == GameScreen::World)
	{
		if (KeyPressEvent::getKeyNumber(e) == Controls::OPEN_CONSOLE)
		{
			if (!m_hud->isRegistered("console"))
			{
				m_gui_console = new GUIEntityConsole();
				m_hud->registerGUIEntity(m_gui_console);
				m_gui_console->open(true);
				e.handled = true;
				return;
			}
		}
	}

	if (e.getEventType() == Event::EventType::KeyPress)
	{
		auto m = static_cast<KeyPressEvent&>(e);


		if (m.getKey() == GLFW_KEY_ESCAPE)
		{
			e.handled = true;
			if (m_game_screen == GameScreen::World)
				proccessWindowEvent(MessageEvent(OpenPause));

			else if (m_game_screen == GameScreen::Pause)
				proccessWindowEvent(MessageEvent(OpenBack));

		}
		if (m.getKey() == Controls::OPEN_INVENTORY && m_game_screen == GameScreen::World)
		{
			e.handled = true;

			if (!m_hud->isRegistered("player"))
			{
				m_gui_player = new GUIEntityPlayer(&m_world->getPlayer());
				m_hud->registerGUIEntity(m_gui_player);
				if (m_world->getPlayer().hasCreative())
				{
					m_gui_creative = new GUIEntityCreativeTab();
					m_hud->registerGUIEntity(m_gui_creative);
				}
			}
			else
			{
				m_gui_player->openInventory(!m_gui_player->isOpenedInventory());

				if (m_gui_player->isOpenedInventory() && m_world->getPlayer().hasCreative())
				{
					m_gui_creative = new GUIEntityCreativeTab();
					m_hud->registerGUIEntity(m_gui_creative);
				}
			}
		}
		//throw item away
		if (m.getKey() == Controls::DROP_ITEM && m_game_screen == GameScreen::World)
		{
			if (m_hud->isRegistered("player"))
			{
				auto& item = m_world->getPlayer().getInventory().itemInHand();
				if (item)
				{
					m_world->getPlayer().throwItem(*m_world->getWorld(), item);
				}
				item = nullptr;
			}
			e.handled = true;
		}
	}
	if (e.getEventType() == Event::EventType::Message)
	{
		return;
	}

	if (e.getEventType() == Event::EventType::WindowResize)
	{
		auto m = static_cast<WindowResizeEvent&>(e);
		m_gui_renderer.setScreenDimensions(m.getWidth(), m.getHeight());
	}


}

void GUILayer::onImGuiRender()
{
	if (!ImGUIopen)
		return;

	if (ImGui::Begin("GUILayer", &ImGUIopen))
	{
		if (ImGui::Button("Reload Dictionary"))
		{
			AppLanguages::loadLanguage(AppLanguages::getCurrentLanguage());
		}
		if (Translator::getNumberOfUnknowns() == 0)
			ImGui::TextColored({ 0.f,1.f,0.f,1.f }, "No unknown entries :D");
		else {
			static std::string s;
			s = "Save Unknown Dictionary Entries (" + std::to_string(Translator::getNumberOfUnknowns()) + ")";
			if (ImGui::Button(s.c_str()))
			{
				ImGuiFileDialog::Instance()->OpenDialog("ChooseDictDump", "Save to", ".lang\0\0", ".");
			}
			// display
			if (ImGuiFileDialog::Instance()->FileDialog("ChooseDictDump"))
			{
				// action if OK
				if (ImGuiFileDialog::Instance()->IsOk == true)
				{
					std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
					Translator::saveUnknownEntries(filePathName.c_str());
				}
				// close
				ImGuiFileDialog::Instance()->CloseDialog("ChooseDictDump");
			}
		}
	}
	ImGui::End();
}
