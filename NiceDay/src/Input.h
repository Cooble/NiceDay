#pragma once
class Input
{
public:
	bool isKeyPressed(int button);
	bool isMousePressed(int button);
	std::pair<float,float> getMouseLocation();

};

