#include "GUILayer.h"
#include "gui/GUIContext.h"
#include "event/WindowEvent.h"
#include "core/App.h"
#include "gui/MainWindow.h"
#include "event/MessageEvent.h"
#include "WorldLayer.h"
#include "core/AppGlobals.h"
#include "CommonMessages.h"
#include "gui/window_messeages.h"
#include "event/KeyEvent.h"
#include "GLFW/glfw3.h"
#include "gui/GUIEntityPlayer.h"
#include "world/entity/EntityPlayer.h"
#include "world/entity/EntityAllocator.h"
#include "event/SandboxControls.h"
#include "gui/GUIEntityPlayer.h"
#include "core/Stats.h"
#include "gui/GUIEntityCreativeTab.h"

GUILayer::GUILayer()
	:m_gui_creative(nullptr)
{
	ND_PROFILE_METHOD();
	m_bound_func = std::bind(&GUILayer::consumeWindowEvent, this, std::placeholders::_1);
	m_gui_context = GUIContext::create();
	m_gui_renderer.setContext(&GUIContext::get());
	m_gui_renderer.m_font_material = FontMatLib::getMaterial("res/fonts/andrew_big.fnt");
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
	m_gui_renderer.setItemAtlas(m_world->getItemAtlas().getSize(),
	                            Texture::create(
		                            TextureInfo("res/images/itemAtlas/atlas.png").
		                            filterMode(TextureFilterMode::NEAREST)));
}

void GUILayer::updateWorldList()
{
	WorldsProvider::get().rescanWorlds();
	m_play_window->setWorlds(WorldsProvider::get().getAvailableWorlds());
}

void GUILayer::proccessWindowEvent(const MessageEvent& e)
{
	auto worldData = (WindowMessageData::World *)e.getData();
	switch (e.getID())
	{
	case WindowMess::MenuPlay:
		{
			GUIContext::get().destroyWindow(m_main_window->id);
			m_main_window = nullptr;
			m_play_window = new PlayWindow(m_bound_func);
			updateWorldList();
			GUIContext::get().openWindow(m_play_window);
		}
		break;
	case WindowMess::MenuPlayWorld:
		{
			GUIContext::get().destroyWindow(m_play_window->id);
			m_play_window = nullptr;

			m_game_screen = GameScreen::World;
			m_world->loadWorld(worldData->worldName, false);

			m_hud = new HUD();
			GUIContext::get().openWindow(m_hud);
			m_background_enable = false;
		}
		break;
	case WindowMess::MenuGenerateWorld:
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

			GUIContext::get().destroyWindow(m_play_window->id);
			m_play_window = nullptr;

			m_game_screen = GameScreen::World;
			m_background_enable = false;
			m_world->loadWorld(worldData->worldName, true);
			m_hud = new HUD();
			GUIContext::get().openWindow(m_hud);
		}
		break;
	case WindowMess::MenuDeleteWorld:
		WorldsProvider::get().deleteWorld(std::string(worldData->worldName));
		updateWorldList();
		break;
	case WindowMess::MenuExit:
		App::get().fireEvent(WindowCloseEvent());
		break;
	case WindowMess::MenuBack:
		{
			if (m_game_screen == GameScreen::GUI)
			{
				if (m_play_window)
					GUIContext::get().destroyWindow(m_play_window->id);
				if (m_controls_window)
					GUIContext::get().destroyWindow(m_controls_window->id);
				m_play_window = nullptr;
				m_controls_window = nullptr;
				m_main_window = new MainWindow(m_bound_func);
				GUIContext::get().openWindow(m_main_window);
			}
			else if (m_game_screen == GameScreen::Pause)
			{
				m_game_screen = GameScreen::World;
				m_world->pause(false);
				GUIContext::get().closeWindow(m_pause_window->id);
			}
		}
		break;

	case WindowMess::MenuSettings:
		{
			if (m_game_screen == GameScreen::GUI)
			{
				GUIContext::get().destroyWindow(m_main_window->id);
				m_main_window = nullptr;
				m_controls_window = new ControlsWindow(m_bound_func);
				GUIContext::get().openWindow(m_controls_window);
			}
			else if (m_game_screen == GameScreen::Pause)
			{
				/*m_game_screen = GameScreen::World;
				m_world->pause(false);
				GUIContext::get().closeWindow(m_pause_window->id);*/
			}
		}
		break;
	case WindowMess::WorldQuit:
		{
			ND_INFO("World unloading:::");
			m_world->onDetach();
			m_world->onAttach();
			ND_INFO("World unloading:::done");

			GUIContext::get().destroyWindow(m_hud->id);
			GUIContext::get().closeWindow(m_pause_window->id);

			m_play_window = nullptr;
			m_main_window = new MainWindow(m_bound_func);
			m_background_enable = true;
			m_game_screen = GameScreen::GUI;
			GUIContext::get().openWindow(m_main_window);
		}
		break;
	}
}

void GUILayer::consumeWindowEvent(const MessageEvent& e)
{
	m_window_event_buffer.push_back(e);
}

void GUILayer::onAttach()
{
	ND_PROFILE_METHOD();
	m_game_screen = GameScreen::GUI;
	GUIContext::setContext(m_gui_context);
	m_main_window = new MainWindow(m_bound_func);
	m_pause_window = new PauseWindow(m_bound_func);
	GUIContext::get().getWindows().push_back(m_main_window);
}

void GUILayer::onDetach()
{
	GUIContext::get().destroyWindows();
}

static int isprofiling = 0;

static int profileIdx = 0;

void GUILayer::onUpdate()
{
	if(isprofiling)
	{
		isprofiling--;
		if(isprofiling==0)
		{
			ND_PROFILE_END_SESSION();
		}
	}
	GUIContext::setContext(m_gui_context);
	GUIContext::get().onUpdate();
	for (auto& event : m_window_event_buffer)
	{
		proccessWindowEvent(event);
	}
	m_window_event_buffer.clear();
}

void GUILayer::onRender()
{
	ND_PROFILE_METHOD();
	m_renderer.begin();
	if (m_background_enable)
		m_renderer.submitTextureQuad({-1, -1.f, 0}, {2.f, 2}, UVQuad::elementary(), m_background);
	m_renderer.push(
		glm::translate(
			glm::mat4(1.f),
			{-1.f, -1.f, 0}));
	m_renderer.push(
		glm::scale(
			glm::mat4(1.f),
			{2.f / App::get().getWindow()->getWidth(), 2.f / App::get().getWindow()->getHeight(), 1}));
	m_gui_renderer.render(m_renderer);
	m_renderer.flush();
	m_renderer.pop();
	m_renderer.pop();
}


void GUILayer::onEvent(Event& e)
{
	
	if (e.getEventType() == Event::EventType::KeyPress)
	{
		auto m = static_cast<KeyPressEvent&>(e);
		
		if (m.getKey() == GLFW_KEY_P)//profile 60ticks
		{
			if(isprofiling==0)
			{
				isprofiling = 60;
				ND_PROFILE_BEGIN_SESSION("snippet"+std::to_string(profileIdx), "snippet" + std::to_string(profileIdx)+".json");
				profileIdx++;
			}
			
		}
		if (m.getKey() == GLFW_KEY_ESCAPE)
		{
			e.handled = true;

			if (m_game_screen == GameScreen::World)
			{
				m_game_screen = GameScreen::Pause;
				m_world->pause(true);
				GUIContext::get().openWindow(m_pause_window);
			}
			else if (m_game_screen == GameScreen::Pause)
			{
				m_game_screen = GameScreen::World;
				m_world->pause(false);
				GUIContext::get().closeWindow(m_pause_window->id);
			}
		}
		if (m.getKey() == Controls::OPEN_INVENTORY && m_game_screen == GameScreen::World)
		{
			e.handled = true;

			if (!m_hud->isRegistered("player")) {
				m_gui_player = new GUIEntityPlayer(&m_world->getPlayer());
				m_hud->registerGUIEntity(m_gui_player);
				if(m_world->getPlayer().hasCreative())
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

	bool flipped = e.isInCategory(Event::EventCategory::Mouse);
	if (flipped)
		static_cast<MouseMoveEvent&>(e).flipY(App::get().getWindow()->getHeight());
	GUIContext::setContext(m_gui_context);
	GUIContext::get().onEvent(e);
	if (flipped)
		static_cast<MouseMoveEvent&>(e).flipY(App::get().getWindow()->getHeight());
}
