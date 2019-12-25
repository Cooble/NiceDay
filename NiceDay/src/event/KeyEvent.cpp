#include "ndpch.h"
#include "KeyEvent.h"
#include "GLFW/glfw3.h"


bool KeyPressEvent::isControlPressed() const
{
	return m_mods & GLFW_MOD_CONTROL;
}

bool KeyPressEvent::isShiftPressed() const
{
	return m_mods & GLFW_MOD_SHIFT;

}

bool KeyPressEvent::isAltPressed() const
{
	return m_mods & GLFW_MOD_ALT;
}
