#pragma once
#include "Window.h"
#include "Input.h"
#include "layer/LayerStack.h"
#include "event/MouseEvent.h"
#include "event/WindowEvent.h"
#include "Scheduler.h"

#define ND_SCHED App::get().getScheduler()

class ImGuiLayer;

class App
{
public:
	App(int width=1280, int height=720, const std::string& title="ND_ENGINE");
	virtual ~App();

	void start();
	void stop();

	virtual bool onWindowClose(WindowCloseEvent& e);

	void fireEvent(Event& e);

	inline static App& get() { return *s_Instance; }
	inline Window* getWindow() { return m_Window; }
	inline Input& getInput() { return m_Input; }
	inline LayerStack& getLayerStack() { return m_LayerStack; }
	inline Scheduler& getScheduler() { return m_scheduler; }

	// return target ticks per second (not actual)
	inline int getTPS() const{ return m_target_tps; }


	//=====telemetry====

	inline float getFPS() const{ return m_fps; }
	inline int getTickMillis() const { return m_tel_tick_millis; }
	inline int getRenderMillis() const { return m_tel_render_millis; }
	inline int getUpdatesPerFrame() { return m_tel_updates_per_frame; }

private:
	static App* s_Instance;
	void init(int width,int height,const std::string& title);
	void update();
	void render();
	int m_tel_updates_per_frame;
protected:
	int current_fps=0;
	long long lastFPSMillis;

	float m_fps;
	int m_tel_tick_millis;
	int m_tel_render_millis;
	int m_target_tps=60;
	bool m_imgui_enable = true;

	Window* m_Window;
	Input m_Input;
	LayerStack m_LayerStack;
	ImGuiLayer* m_ImGuiLayer=nullptr;
	Scheduler m_scheduler;
	bool m_running=false;
	
};

class ImGuiLayer : public Layer
{
public:
	ImGuiLayer();
	~ImGuiLayer() = default;
	void onAttach();
	void onDetach();
	void begin();
	void end();
	void onImGuiRender();
	void onUpdate() override;
	void updateTelemetry();
	void drawTelemetry();
};

