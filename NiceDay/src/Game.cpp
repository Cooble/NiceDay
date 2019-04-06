#include "ndpch.h"
#include "Game.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <chrono>

#define ND_TPS_MS 20

Game* Game::s_Instance = nullptr;

Game::Game()
{
	s_Instance = this;
}

void Game::init()
{
	m_Window = new Window(1280, 720, "NiceDay");
	ND_INFO("Window created");
}

void Game::start()
{
	m_running = true;
	int over_millis = 1000 / ND_TPS_MS;
	auto lastTime = std::chrono::system_clock::now();
	while (!m_Window->shouldClose() && m_running) {
		glfwPollEvents();
		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count() > over_millis) {
			lastTime = now;
			update();
		}
		render();
	}
}

void Game::update() {
	ND_INFO("Tick");

}
void Game::render() {

}
void Game::stop() {
	m_running = false;

}


Game::~Game()
{
	delete m_Window;
}
