#include "ndpch.h"
#include "App.h"
#include "event/Event.h"
#include "event/WindowEvent.h"
#include <chrono>
#include "graphics/GContext.h"
#include "graphics/Effect.h"
#include "core/FakeWindow.h"
#include "GLFW/glfw3.h"
#include "core/AppGlobals.h"
#include "lua/LuaLayer.h"
#include "event/ControlMap.h"
#include "event/KeyEvent.h"
#include "event/MessageEvent.h"
#include "audio/SoundLayer.h"
#include "ImGuiLayer.h"
#include "script/MonoLayer.h"


#define BIND_EVENT_FN(x) std::bind(&App::x, &App::get(), std::placeholders::_1)


App* App::s_Instance = nullptr;

static FakeWindow* fakeWindow;

App::App()
	: m_scheduler(200),
	  m_dbuff_stackalloc(1000000) //1MB
{
	m_thread_id = std::this_thread::get_id();
	ASSERT(s_Instance == nullptr, "Instance of game already exists!")
	s_Instance = this;
}

App::App(const AppInfo& info):App()
{
	init(info);
}

static void eventCallback(Event& e);
static void physicalWindowCallback(Event& e);

void App::init(const AppInfo& info)
{
	m_info = info;
	m_io.enableSCENE = info.enableSCENE;
	m_io.enableIMGUI = info.enableIMGUI;
	m_io.enableSOUND = info.enableSOUND;
	m_settings = new NBT();
	NBT::loadFromFile("settings.json", *m_settings);
	if (m_settings->type == NBT::T_NULL) {
		m_settings->access_map("INFO") = "Application temporary file";
	}
	ControlMap::init();
	Log::init();
	m_Window = new Window(info.width, info.height, info.title);
	m_Input = new RealInput(m_Window);
	if (m_io.enableSCENE) {
		m_fakeWindow = new FakeWindow(m_Window, 100, 100, "FakeWindow");
		m_FakeInput = new FakeInput(m_fakeWindow, m_Input);
		m_defaultWindow = m_fakeWindow;
		m_defaultInput = m_FakeInput;
	}
	else {
		m_defaultWindow = m_Window;
		m_defaultInput = m_Input;
	}
	fakeWindow = m_fakeWindow;
	GContext::init(Renderer::getAPI());
	Effect::init();
	m_Window->setEventCallback(m_io.enableSCENE ? physicalWindowCallback : eventCallback);
	m_lua_layer = new LuaLayer();
	m_LayerStack.pushLayer(m_lua_layer);
	m_mono_layer = new MonoLayer();
	m_LayerStack.pushLayer(m_mono_layer);
	if (m_io.enableSOUND)
		m_LayerStack.pushLayer(new SoundLayer());
	if (m_io.enableIMGUI)
	{
		m_ImGuiLayer = new ImGuiLayer();
		m_LayerStack.pushOverlay(m_ImGuiLayer);
	}

}

App::~App()
{

	delete m_settings;
	delete m_Window;
}

bool App::onWindowClose(WindowCloseEvent& e)
{
	stop();
	return true;
}

static void eventCallback(Event& e)
{
	LayerStack& stack = App::get().getLayerStack();
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(onWindowClose));
	for (auto it = stack.end(); it != stack.begin();)
	{
		(*--it)->onEvent(e);
		if (e.handled)
			break;
	}
}
static void physicalWindowCallback(Event& e)
{
	fakeWindow->convertEvent(e);
	if (e.handled)
		return;
	eventCallback(e);
}

void App::fireEvent(Event& e)
{
	eventCallback(e);
}

uint64_t nowTime()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void App::start()
{
	ND_TRACE("Game started");

	using namespace std::chrono;
	m_running = true;
	auto lastTime = nowTime();
	int millisPerTick = 1000 / m_target_tps;
	uint64_t accomodator = 0;
	auto lastRenderTime = nowTime();
	while (m_running)
	{
		auto now = nowTime();
		accomodator += now - lastTime;
		lastTime = now;
		m_tel_updates_per_frame = accomodator / millisPerTick;
		if (m_tel_updates_per_frame > 20)
			accomodator = 0; //lets slow the game to catch up
		int maxMil = 0;
		while (accomodator >= millisPerTick)
		{
			accomodator -= millisPerTick;

			auto tt = nowTime();
			update();
			maxMil = std::max((int)(nowTime() - tt), maxMil);
		}
		m_tel_tick_millis = maxMil;

		//if(nowTime()-lastRenderTime<1000/m_target_tps)
		if (nowTime() - lastRenderTime < 1000 / 50)
			continue; //skip render
		now = nowTime();
		lastRenderTime = now;
		if(!m_Window->isIconified())
			render();
		m_tel_render_millis = nowTime() - now;
		static int totalMil = 0;
		totalMil += m_tel_render_millis;
		static int divi = 0;
		if (divi++ == 60)
		{
			divi = 0;
			totalMil /= 61;
			totalMil = 0;
		}

		current_fps++;
		auto noww = nowTime();
		if (noww - lastFPSMillis > 500)
		{
			m_fps = current_fps * 2;
			current_fps = 0;
			lastFPSMillis = nowTime();
		}
	}

	ND_PROFILE_BEGIN_SESSION("end", "end.json");
	for (Layer* l : m_LayerStack)
		l->onDetach();

	m_Window->close();
	ND_TRACE("Saving settings.json");
	NBT::saveToFile("settings.json", *m_settings);
	ND_TRACE("Game quitted");
	ND_PROFILE_END_SESSION();
}

void App::update()
{
	m_Window->pollEvents();
	m_Input->update();
	for (Layer* l : m_LayerStack)
		l->onUpdate();
	m_scheduler.update();
	m_LayerStack.popPending();
	m_dbuff_stackalloc.swapBuffers();
}

void App::render()
{
	ND_PROFILE_METHOD();

	//ND_PROFILE_CALL(m_Window->swapBuffers());
	m_Window->swapBuffers();
	Renderer::getDefaultFBO()->bind();
	Renderer::getDefaultFBO()->clear(BuffBit::COLOR|BuffBit::DEPTH);
	for (Layer* l : m_LayerStack)
		l->onRender();

	m_Window->getFBO()->bind();
	if (m_info.enableSCENE)
		m_Window->getFBO()->clear(BuffBit::COLOR | BuffBit::DEPTH);
	if (m_info.enableIMGUI)
	{
		ND_PROFILE_SCOPE("imgui app render");
		m_ImGuiLayer->begin();

		//render the first main imguiwindow
		if(m_info.enableSCENE)
			m_fakeWindow->renderView();
		
		for (Layer* l : m_LayerStack)
			l->onImGuiRender();
		m_ImGuiLayer->end();

		//prepare window for next
		if (m_info.enableSCENE)
			m_fakeWindow->swapBuffers();
	}
}

void App::stop()
{
	m_running = false;
}
