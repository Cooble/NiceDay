#include "ndpch.h"
#include "Game.h"
#include "event/Event.h"
#include "event/WindowEvent.h"
#include "layer/MainLayer.h"
#include "layer/ImGuiLayer.h"
#include "layer/WorldLayer.h"
#include <chrono>

#define BIND_EVENT_FN(x) std::bind(&Game::x, &Game::get(), std::placeholders::_1)

#define ND_TPS_MS 30

Game* Game::s_Instance = nullptr;

Game::Game()
{
	ASSERT(s_Instance == nullptr, "Instance of game already exists!")
		s_Instance = this;
}

bool Game::onWindowClose(WindowCloseEvent& e){
	ND_INFO("Window was closed");
	stop();
	return true;

}

static void eventCallback(Event& e) {

	LayerStack& stack = Game::get().getLayerStack();
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(onWindowClose));

	for (auto it = stack.end(); it != stack.begin(); )
	{
		(*--it)->onEvent(e);
		if (e.handled)
			break;
	}
}

void Game::init()
{
	m_Window = new Window(1500, 720, "NiceDay");
	m_Window->setEventCallback(eventCallback);
	auto l = new MainLayer();
	m_LayerStack.PushLayer(l);
	m_ImGuiLayer = new ImGuiLayer();
	m_LayerStack.PushOverlay(m_ImGuiLayer);
	m_LayerStack.PushLayer(new WorldLayer());
}


void Game::start()
{
	ND_TRACE("Game started");
	m_running = true;
	int over_millis = 1000 / ND_TPS_MS;
	auto lastTime = std::chrono::system_clock::now();
	while (m_running) {
		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count() > over_millis) {
			lastTime = now;
			update();
		}
		render();

	}
	for (Layer* l : m_LayerStack)
		l->onDetach();

	m_Window->close();
	ND_TRACE("Game quitted");
}

void Game::update() {
	for (Layer* l : m_LayerStack)
		l->onUpdate();



}
void Game::render() {
	m_Window->update();
	for (Layer* l : m_LayerStack)
		l->onRender();

	m_ImGuiLayer->begin();
	for (Layer* l : m_LayerStack)
		l->onImGuiRender();
	m_ImGuiLayer->end();

}
void Game::stop() {
	m_running = false;
}

Game::~Game()
{
	delete m_Window;
}
