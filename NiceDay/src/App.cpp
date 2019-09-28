#include "ndpch.h"
#include "App.h"
#include "event/Event.h"
#include "event/WindowEvent.h"
#include <chrono>
#include "graphics/GContext.h"
#include "Stats.h"
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include "graphics/Effect.h"

#define BIND_EVENT_FN(x) std::bind(&App::x, &App::get(), std::placeholders::_1)


App* App::s_Instance = nullptr;

App::App(int width, int height, const std::string& title)
{
	ASSERT(s_Instance == nullptr, "Instance of game already exists!")
	s_Instance = this;
	init(width,height,title);
}
static void eventCallback(Event& e);
void App::init(int width, int height, const std::string& title)
{
	Log::Init();
	m_Window = new Window(width, height, title);
	GContext::init(Renderer::getAPI());
	Effect::init();
	m_Window->setEventCallback(eventCallback);

	m_ImGuiLayer = new ImGuiLayer();
	m_LayerStack.PushOverlay(m_ImGuiLayer);
}

App::~App()
{
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
	using namespace std::chrono;
	ND_TRACE("Game started");
	m_running = true;
	auto lastTime = nowTime();
	int millisPerTick = 1000 / m_target_tps;
	uint64_t accomodator = 0;
	while (m_running)
	{
		auto now = nowTime();
		accomodator += now - lastTime;
		lastTime = now;
		Stats::updates_per_frame = accomodator / millisPerTick;
		if (Stats::updates_per_frame > 20)
			accomodator = 0;//lets slow the game to catch up
		int maxMil = 0;
		while (accomodator >= millisPerTick)
		{
			accomodator -= millisPerTick;
			
			auto tt = nowTime();
			update();
			maxMil = std::max((int)(nowTime() - tt),maxMil);
		}
		m_tick_millis = maxMil;
		now = nowTime();
		render();
		m_render_millis = nowTime() - now;

		current_fps++;
		auto noww = nowTime();
		if (noww - lastFPSMillis > 500)
		{
			m_fps = current_fps * 2;
			current_fps = 0;
			lastFPSMillis = nowTime();
		}
	}
	for (Layer* l : m_LayerStack)
		l->onDetach();

	m_Window->close();
	ND_TRACE("Game quitted");
}

void App::update()
{
	m_Input.update();
	for (Layer* l : m_LayerStack)
		l->onUpdate();
	m_scheduler.update();
}

void App::render()
{
	m_Window->update();
	for (Layer* l : m_LayerStack)
		l->onRender();

	m_ImGuiLayer->begin();
	for (Layer* l : m_LayerStack)
		l->onImGuiRender();
	m_ImGuiLayer->end();
}

void App::stop()
{
	m_running = false;
}


//========IMGUILAYER========================


ImGuiLayer::ImGuiLayer()
	: Layer("ImGuiLayer")
{
}

void ImGuiLayer::onAttach()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	/*if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}*/

	App& app = App::get();
	auto* window = static_cast<GLFWwindow*>(app.getWindow()->getWindow());

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410");
}

void ImGuiLayer::onDetach()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiLayer::begin()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiLayer::end()
{
	ImGuiIO& io = ImGui::GetIO();
	App& app = App::get();
	io.DisplaySize = ImVec2(app.getWindow()->getWidth(), app.getWindow()->getHeight());

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	/*if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}*/
}

void ImGuiLayer::onImGuiRender()
{
	//static bool show = true;
	//ImGui::ShowDemoWindow(&show);
}


