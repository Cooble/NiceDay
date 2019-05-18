#pragma once
#include "Window.h"
#include "Input.h"
#include "layer/LayerStack.h"
#include "event/MouseEvent.h"
#include "event/WindowEvent.h"
#include "layer/ImGuiLayer.h"

constexpr int TARGET_TPS=60;

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
	inline float getFPS() const{ return m_fps; }
	inline int getTargetTPS() const{ return m_target_tps; }

private:
	static Game* s_Instance;
	int current_fps=0;
	long long lastFPSMillis;

	float m_fps;
	int m_target_tps;

	Window* m_Window;
	Input m_Input;
	LayerStack m_LayerStack;
	ImGuiLayer* m_ImGuiLayer;
	bool m_running=false;


};

