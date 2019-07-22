#pragma once
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
	std::pair<float,float> getMouseLocation();

};

