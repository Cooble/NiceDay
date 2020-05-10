#include "ndpch.h"
#include "KeyEvent.h"
#include "GLFW/glfw3.h"


bool KeyPressEvent::isControlPressed() const
{
	return m_mods & GLFW_MOD_CONTROL;
}

int KeyPressEvent::getKeyNumber(Event& e)
{
	if (e.getEventType() != EventType::KeyPress)
		return -1;
	auto m = static_cast<KeyPressEvent*>(&e);
	return m->getKey();
}

int KeyTypeEvent::getKeyNumber(Event& e)
{
	if (e.getEventType() != EventType::KeyType)
		return -1;
	auto m = static_cast<KeyTypeEvent*>(&e);
	return m->getKey();
}

bool KeyPressEvent::isShiftPressed() const
{
	return m_mods & GLFW_MOD_SHIFT;

}

bool KeyPressEvent::isAltPressed() const
{
	return m_mods & GLFW_MOD_ALT;
}
