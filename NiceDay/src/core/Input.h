#pragma once

/**
 * Keeps track of all user mouse and keyboard inputs generated from GLFWWindow
 * needs update() to check each tick for new data
 */
class Input
{
private:
	std::vector<int8_t> m_keys;
	int8_t& getKey(int button);
public:
	void update();
	bool isKeyPressed(int button);
	bool isKeyFreshlyPressed(int button);
	bool isKeyFreshlyReleased(int button);
	bool isMousePressed(int button);
	glm::vec2 getMouseLocation();

};

