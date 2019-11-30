#include "GUILayer.h"
#include "gui/GUIContext.h"
#include "event/WindowEvent.h"
#include "core/App.h"
#include "gui/MainWindow.h"
#include "event/MessageEvent.h"
#include "WorldLayer.h"
#include "core/AppGlobals.h"
#include "CommonMessages.h"

GUILayer::GUILayer()
{
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

void GUILayer::onAttach()
{
	GUIContext::setContext(m_gui_context);
	GUIContext::get().getWindows().push_back(new MainWindow());
}

void GUILayer::onDetach()
{
}


void GUILayer::onUpdate()
{
	GUIContext::setContext(m_gui_context);
	GUIContext::get().onUpdate();
}

void GUILayer::onRender()
{
	m_renderer.begin();
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
		auto m = static_cast<MessageEvent&>(e);
		if (strcmp("PlayBtnEvent", m.getTitle()) == 0)
		{
			App::get().getLayerStack().PopLayerEventually(this);
			App::get().getLayerStack().PushLayerEventually(new WorldLayer());
			return;
		}
		else if (strcmp("PlayNewBtnEvent", m.getTitle()) == 0)
		{
			App::get().getLayerStack().PopLayerEventually(this);
			AppGlobals::get().nbt.set("set.newWorld", true);
			App::get().getLayerStack().PushLayerEventually(new WorldLayer());
		}
		else if (strcmp("PlayWorld", m.getTitle()) == 0)
		{
			auto mm = (CommonMessages::PlayMessage*)m.getData();
			App::get().getLayerStack().PopLayerEventually(this);
			AppGlobals::get().nbt.set("set.worldName", mm->worldName);
			App::get().getLayerStack().PushLayerEventually(new WorldLayer());
		}
		else if (strcmp("Play", m.getTitle()) == 0)
		{
			auto mm = (CommonMessages::PlayMessage*)m.getData();

			App::get().getLayerStack().PopLayerEventually(this);

			
			AppGlobals::get().nbt.set("set.worldName", mm->worldName);

			App::get().getLayerStack().PushLayerEventually(new WorldLayer());
		}


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
