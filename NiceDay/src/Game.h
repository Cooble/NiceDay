#pragma once
#include "Window.h"
#include "Input.h"
#include "event/MouseEvent.h"
class Game
{

public:
	Game();
	~Game();

	void init();
	void start();
	void update();
	void render();
	void stop();
	bool consumeEsc(MousePressEvent&);

	inline static Game& get() { return *s_Instance; }
	inline Window& getWindow() { return *m_Window; }
	inline Input& getInput() { return m_Input; }

private:
	static Game* s_Instance;

	Window* m_Window;
	Input m_Input;
	bool m_running;


};

