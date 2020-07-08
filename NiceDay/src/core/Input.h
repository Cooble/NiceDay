#pragma once

/**
 * Keeps track of all user mouse and keyboard inputs generated from GLFWWindow
 * needs update() to check each tick for new data
 */
class Input
{
public:
	virtual void update(){}
	virtual bool isKeyPressed(int button) = 0;
	virtual bool isKeyFreshlyPressed(int button) = 0;
	virtual bool isKeyFreshlyReleased(int button) = 0;
	virtual bool isMousePressed(int button) = 0;
	virtual glm::vec2 getMouseLocation() = 0;
};

