#pragma once
#include "Window.h"
#include "Input.h"
#include "layer/LayerStack.h"
#include "event/MouseEvent.h"
#include "event/WindowEvent.h"
#include "Scheduler.h"
#include "memory/StackAllocator.h"

#define ND_SCHED App::get().getScheduler()

// allocates string on doublebuffered stack
// this string is valid for 2 ticks
#define ND_TEMP_STRING(x) App::get().getBufferedAllocator().allocateString((x))
#define ND_TEMP_EMPLACE(type, ...)  App::get().getBufferedAllocator().emplace<type>(__VA_ARGS__)
#define ND_IMGUI_VIEW(name,t) App::get().getImGui()->renderView((name),(t))
#define ND_IMGUI_VIEW_PROXY(name,t) App::get().getImGui()->renderViewProxy((name),(t))
class ImGuiLayer;
class LuaLayer;
class NBT;
class WindowTemplate;
class FakeWindow;
class FakeInput;
class MonoLayer;



class App
{

protected:
	App();
public:
	struct IO
	{
		bool enableIMGUI = true;
		bool enableSOUND = true;
		bool enableSCENE = false;
		bool enableMONO = false;
	};
	struct AppInfo
	{
		int width = 1280, height = 720;
		std::string title = "ND_ENGINE";
		IO io;
	};
	App(const App::AppInfo& info);
	virtual ~App();

	void start();
	void stop();

	virtual bool onWindowClose(WindowCloseEvent& e);

	void fireEvent(Event& e);

	static App& get() { return *s_Instance; }
	WindowTemplate* getWindow() { return m_defaultWindow; }
	Window* getPhysicalWindow() { return m_Window; }
	Input& getInput() { return *m_defaultInput; }
	Input& getPhysicalInput() { return *m_Input; }
	LayerStack& getLayerStack() { return m_LayerStack; }
	Scheduler& getScheduler() { return m_scheduler; }
	DoubleBuffStackAllocator& getBufferedAllocator() { return m_dbuff_stackalloc; }
	LuaLayer* getLua() { return m_lua_layer; }
	ImGuiLayer* getImGui() { return m_ImGuiLayer; }
	NBT& getSettings() { return *m_settings; }

	// return target ticks per second (not actual)
	int getTPS() const{ return m_target_tps; }


	//=====telemetry====

	float getFPS() const{ return m_fps; }
	int getTickMillis() const { return m_tel_tick_millis; }
	int getRenderMillis() const { return m_tel_render_millis; }
	int getUpdatesPerFrame() { return m_tel_updates_per_frame; }
	std::thread::id getMainThreadID() { return m_thread_id; }

	const IO& getIO() const { return m_io; }
	
private:
	IO m_io;
	static App* s_Instance;
	std::thread::id m_thread_id;
	void update();
	void render();
	int m_tel_updates_per_frame;
protected:
	NBT* m_settings;
	void init(const AppInfo& info);
	int current_fps=0;
	long long lastFPSMillis;

	float m_fps;
	int m_tel_tick_millis;
	int m_tel_render_millis;
	int m_target_tps=60;

	AppInfo m_info;

	Window* m_Window;
	FakeWindow* m_fakeWindow;
	WindowTemplate* m_defaultWindow;

	Input* m_Input;
	FakeInput* m_FakeInput;
	Input* m_defaultInput;

	LayerStack m_LayerStack;
	ImGuiLayer* m_ImGuiLayer=nullptr;
	LuaLayer* m_lua_layer=nullptr;
	MonoLayer* m_mono_layer=nullptr;
	Scheduler m_scheduler;
	DoubleBuffStackAllocator m_dbuff_stackalloc;
	bool m_running=false;
	
};

//Debug settings variables -> loaded and saved to app.json


