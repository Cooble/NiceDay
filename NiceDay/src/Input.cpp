#include "ndpch.h"
#include "Input.h"
#include "Game.h"
#include "Window.h"


int8_t& Input::getKey(int button)
{
	if(button>=m_keys.size())
	{
		auto lastSize = m_keys.size();
		m_keys.resize(button + 1);
		for (int i = 0; i < (button+1)-lastSize; ++i)//clear all new keys
			m_keys[i + lastSize] = 0;
		m_keys[button] = 0;
	}
	return m_keys[button];
}

void Input::update()
{
	for (int i = 0; i < m_keys.size(); ++i)
	{
		auto& k = getKey(i);
		if (isKeyPressed(i)) {
			if (k < 127)
				++k;
		}
		else if(k>0)
			k = -1;
		else k = 0;
	}
}

bool Input::isKeyPressed(int button)
{
	getKey(button);//save this button to vector
	GLFWwindow* w = Game::get().getWindow()->getWindow();
	auto state = glfwGetKey(w, button);
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}
bool Input::isKeyFreshlyPressed(int button)
{
	return getKey(button) == 1;
}
bool Input::isKeyFreshlyReleased(int button)
{
	return getKey(button) == -1;
}

bool Input::isMousePressed(int button)
{
	GLFWwindow* w = Game::get().getWindow()->getWindow();
	auto state = glfwGetMouseButton(w, button);
	return state==GLFW_PRESS;
}

std::pair<float, float> Input::getMouseLocation()
{
	GLFWwindow* w = Game::get().getWindow()->getWindow();
	double x, y;
	glfwGetCursorPos(w, &x, &y);
	return std::make_pair<>((float)x,(float)y);
}
