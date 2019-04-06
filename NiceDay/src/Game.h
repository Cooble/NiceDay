#pragma once
#include "Window.h"
class Game
{
private:
	static Game* s_Instance;
private:
	Window* m_Window;
	bool m_running;


public:
	Game();
	void init();
	void start();
	void update();
	void render();
	void stop();
	~Game();
	inline static Game& get() { return *s_Instance; }

};

