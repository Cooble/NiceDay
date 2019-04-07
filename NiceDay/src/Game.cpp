#include "ndpch.h"
#include "Game.h"
#include "event/Event.h"
#include <chrono>

#define BIND_EVENT_FN(x) std::bind(&Game::x, &Game::get(), std::placeholders::_1)

#define ND_TPS_MS 30

Game* Game::s_Instance = nullptr;

Game::Game()
{
	ASSERT(s_Instance==nullptr,"Instance of game already exists!")
	s_Instance = this;
}

bool Game::consumeEsc(MousePressEvent& e) {
	ND_INFO("I have consumed {0}",e.toString());
	return true;

}
static void callee(Event& e) {
	//ND_INFO(e.toString());
	EventDispatcher dis(e);
	dis.dispatch<MousePressEvent>(BIND_EVENT_FN(consumeEsc));

	if (e.getEventType() == Event::EventType::WindowClose) {
		Game::get().stop();
	}
}
void Game::init()
{
	m_Window = new Window(1280, 720, "NiceDay");
	m_Window->setEventCallback(callee);
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
		m_Window->update();

	}
	m_Window->close();
	ND_TRACE("Game quitted");
}

void Game::update() {
}
void Game::render() {

}
void Game::stop() {
	m_running = false;
}

Game::~Game()
{
}
