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
class ImGuiLayer;
class LuaLayer;
class NBT;

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
	inline DoubleBuffStackAllocator& getBufferedAllocator() { return m_dbuff_stackalloc; }
	inline LuaLayer* getLua() { return m_lua_layer; }

	// return target ticks per second (not actual)
	inline int getTPS() const{ return m_target_tps; }


	//=====telemetry====

	inline float getFPS() const{ return m_fps; }
	inline int getTickMillis() const { return m_tel_tick_millis; }
	inline int getRenderMillis() const { return m_tel_render_millis; }
	inline int getUpdatesPerFrame() { return m_tel_updates_per_frame; }
	inline std::thread::id getMainThreadID() { return m_thread_id; }

private:
	static App* s_Instance;
	std::thread::id m_thread_id;
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
	LuaLayer* m_lua_layer=nullptr;
	Scheduler m_scheduler;
	DoubleBuffStackAllocator m_dbuff_stackalloc;
	bool m_running=false;
	
};

class ImGuiLayer : public Layer
{
	bool m_imgui_consuming = false;

public:
	ImGuiLayer();
	~ImGuiLayer() = default;
	void onAttach();
	void onDetach();
	void begin();
	void end();
	void onImGuiRender();
	void onUpdate() override;
	void onEvent(Event& e) override;
	void updateTelemetry();
	void drawTelemetry();
	inline bool isEventConsuming() { return m_imgui_consuming; }

	void drawGlobals();
	bool drawNBT(const char* name,NBT& n);
	void drawNBTConst(const char* name,const NBT& n);
};

