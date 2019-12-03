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

GUILayer::GUILayer()
{
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
		GUIContext::get().closeWindows();
		m_main_window = nullptr;
		m_play_window = new PlayWindow(m_bound_func);
		updateWorldList();
		GUIContext::get().openWindow(m_play_window);
		break;
	case WindowMess::MenuPlayWorld:
		GUIContext::get().closeWindows();
		m_world->loadWorld(worldData->worldName, false);
		m_background_enable = false;
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
			GUIContext::get().closeWindows();
			m_background_enable = false;
			m_world->loadWorld(worldData->worldName, true);
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
		GUIContext::get().closeWindows();
		m_play_window = nullptr;
		m_main_window = new MainWindow(m_bound_func);
		GUIContext::get().openWindow(m_main_window);
		break;
	}
}

void GUILayer::consumeWindowEvent(const MessageEvent& e)
{
	m_window_event_buffer.push_back(e);
}

void GUILayer::onAttach()
{
	GUIContext::setContext(m_gui_context);
	m_main_window = new MainWindow(m_bound_func);
	GUIContext::get().getWindows().push_back(m_main_window);
	
}

void GUILayer::onDetach()
{
}


void GUILayer::onUpdate()
{
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
