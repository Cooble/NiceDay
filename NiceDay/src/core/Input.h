#pragma once
#include "event/KeyEvent.h"
#include "event/MouseEvent.h"

namespace nd {
/**
 * Keeps track of all user mouse and keyboard inputs generated from GLFWWindow
 * needs update() to check each tick for new data
 */
class Input
{
public:
	virtual void update()
	{
	}

	virtual bool isKeyPressed(KeyCode button) = 0;
	virtual bool isKeyFreshlyPressed(KeyCode button) = 0;
	virtual bool isKeyFreshlyReleased(KeyCode button) = 0;
	virtual bool isMousePressed(MouseCode button) = 0;
	virtual bool isMouseFreshlyPressed(MouseCode button) = 0;
	virtual bool isMouseFreshlyReleased(MouseCode button) = 0;
	virtual glm::vec2 getDragging() = 0; // should check isMousePressed beforehand


	virtual glm::vec2 getMouseLocation() = 0;


	//just for syntactic sugar
	bool isMousePressed(int button) { return isMousePressed((MouseCode)button); }
	bool isMouseFreshlyPressed(int button) { return isMouseFreshlyPressed((MouseCode)button); }
	bool isMouseFreshlyReleased(int button) { return isMouseFreshlyReleased((MouseCode)button); }
	bool isKeyPressed(int button) { return isKeyPressed((KeyCode)button); }
	bool isKeyFreshlyPressed(int button) { return isKeyFreshlyPressed((KeyCode)button); }
	bool isKeyFreshlyReleased(int button) { return isKeyFreshlyReleased((KeyCode)button); }
};
}
