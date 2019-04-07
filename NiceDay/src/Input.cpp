#include "ndpch.h"
#include "Input.h"
#include "Game.h"
#include "Window.h"


bool Input::isKeyPressed(int button)
{
	GLFWwindow& w = Game::get().getWindow().getWindow();
	auto state = glfwGetKey(&w, button);
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::isMousePressed(int button)
{
	GLFWwindow& w = Game::get().getWindow().getWindow();
	auto state = glfwGetMouseButton(&w, button);
	return state==GLFW_PRESS;
}

std::pair<float, float> Input::getMouseLocation()
{
	GLFWwindow& w = Game::get().getWindow().getWindow();
	double x, y;
	glfwGetCursorPos(&w, &x, &y);
	return std::make_pair<>((float)x,(float)y);
}
