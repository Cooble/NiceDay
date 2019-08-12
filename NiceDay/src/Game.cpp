#include "ndpch.h"
#include "Game.h"
#include "event/Event.h"
#include "event/WindowEvent.h"
#include "layer/MainLayer.h"
#include "layer/ImGuiLayer.h"
#include "layer/WorldLayer.h"
#include <chrono>
#include "graphics/GContext.h"

#define BIND_EVENT_FN(x) std::bind(&Game::x, &Game::get(), std::placeholders::_1)


Game* Game::s_Instance = nullptr;

Game::Game()
{
	ASSERT(s_Instance == nullptr, "Instance of game already exists!")
	s_Instance = this;
}

bool Game::onWindowClose(WindowCloseEvent& e)
{
	stop();
	return true;
}

static void eventCallback(Event& e)
{
	LayerStack& stack = Game::get().getLayerStack();
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(onWindowClose));

	for (auto it = stack.end(); it != stack.begin();)
	{
		(*--it)->onEvent(e);
		if (e.handled)
			break;
	}
}

void Game::fireEvent(Event& e)
{
	eventCallback(e);
}


void Game::init()
{
	m_target_tps = TARGET_TPS;
#ifdef ND_DEBUG
	std::string s = "Niceday - Debug";
#else
	std::string s = "Niceday - Release";
#endif
	m_Window = new Window(1280, 720, s.c_str());
	GContext::get().init(Renderer::getAPI());
	Effect::init();
	Sprite2D::init();
	m_Window->setEventCallback(eventCallback);
	auto l = new MainLayer();
	m_LayerStack.PushLayer(l);
	m_ImGuiLayer = new ImGuiLayer();
	m_LayerStack.PushOverlay(m_ImGuiLayer);
	m_LayerStack.PushLayer(new WorldLayer());
}

uint64_t nowTime()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void Game::start()
{
	using namespace std::chrono;
	ND_TRACE("Game started");
	m_running = true;
	auto lastTime = system_clock::now();
	while (m_running)
	{
		int over_millis = 1000 / m_target_tps;
		auto now = system_clock::now();
		if (duration_cast<milliseconds>(now - lastTime).count() > over_millis)
		{
			lastTime = now;
			update();
			m_tick_millis = duration_cast<milliseconds>(system_clock::now() - now).count();
		}
		render();
		current_fps++;
		auto noww = nowTime();
		if (noww - lastFPSMillis > 100)
		{
			m_fps = current_fps * 10;
			current_fps = 0;
			lastFPSMillis = nowTime();
		}
	}
	for (Layer* l : m_LayerStack)
		l->onDetach();

	m_Window->close();
	ND_TRACE("Game quitted");
}

void Game::update()
{
	m_Input.update();
	for (Layer* l : m_LayerStack)
		l->onUpdate();
	m_scheduler.update();
}

void Game::render()
{
	m_Window->update();
	for (Layer* l : m_LayerStack)
		l->onRender();

	m_ImGuiLayer->begin();
	for (Layer* l : m_LayerStack)
		l->onImGuiRender();
	m_ImGuiLayer->end();
}

void Game::stop()
{
	m_running = false;
}

Game::~Game()
{
	delete m_Window;
}
