#pragma once
#include "Window.h"
#include "Input.h"
#include "layer/LayerStack.h"
#include "event/MouseEvent.h"
#include "event/WindowEvent.h"
#include "layer/ImGuiLayer.h"
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
	bool onWindowClose(WindowCloseEvent&);

	inline static Game& get() { return *s_Instance; }
	inline Window* getWindow() { return m_Window; }
	inline Input& getInput() { return m_Input; }
	inline LayerStack& getLayerStack() { return m_LayerStack; }

private:
	static Game* s_Instance;

	Window* m_Window;
	Input m_Input;
	LayerStack m_LayerStack;
	ImGuiLayer* m_ImGuiLayer;
	bool m_running=false;


};

