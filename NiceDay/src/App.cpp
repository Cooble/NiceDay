#include "ndpch.h"
#include "App.h"
#include "event/Event.h"
#include "event/WindowEvent.h"
#include <chrono>
#include "graphics/GContext.h"
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include "graphics/Effect.h"
#include "GLFW/glfw3.h"
#include "imgui_utils.h"

#define BIND_EVENT_FN(x) std::bind(&App::x, &App::get(), std::placeholders::_1)


App* App::s_Instance = nullptr;

App::App(int width, int height, const std::string& title)
	:m_scheduler(200)
{
	ASSERT(s_Instance == nullptr, "Instance of game already exists!")
	s_Instance = this;
	init(width,height,title);
}
static void eventCallback(Event& e);
void App::init(int width, int height, const std::string& title)
{
	Log::init();
	m_Window = new Window(width, height, title);
	GContext::init(Renderer::getAPI());
	Effect::init();
	m_Window->setEventCallback(eventCallback);
	m_imgui_enable = true;
	if (m_imgui_enable) {
		m_ImGuiLayer = new ImGuiLayer();
		m_LayerStack.PushOverlay(m_ImGuiLayer);
	}
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
			accomodator = 0;//lets slow the game to catch up
		int maxMil = 0;
		while (accomodator >= millisPerTick)
		{
			accomodator -= millisPerTick;
			
			auto tt = nowTime();
			update();
			maxMil = std::max((int)(nowTime() - tt),maxMil);
		}
		m_tel_tick_millis = maxMil;
		
		if(nowTime()-lastRenderTime<1000/m_target_tps)
			continue;//skip render
		now = nowTime();
		lastRenderTime = now;
		render();
		m_tel_render_millis = nowTime() - now;
		static int totalMil = 0;
		totalMil += m_tel_render_millis;
		static int divi = 0;
		if (divi++ == 60) {
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
	for (Layer* l : m_LayerStack)
		l->onDetach();

	m_Window->close();
	ND_TRACE("Game quitted");
}

void App::update()
{
	m_Window->pollEvents();
	m_Input.update();
	for (Layer* l : m_LayerStack)
		l->onUpdate();
	m_scheduler.update();
}

void App::render()
{
	m_Window->swapBuffers();
	for (Layer* l : m_LayerStack)
		l->onRender();
	
	if (m_imgui_enable) {
		m_ImGuiLayer->begin();
		for (Layer* l : m_LayerStack)
			l->onImGuiRender();
		m_ImGuiLayer->end();
	}
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

static int maxValResetDelay = 60 * 2;
static bool openTelemetrics = false;
static bool showTelemetrics = false;

void ImGuiLayer::onImGuiRender()
{
	//static bool show = true;
	//ImGui::ShowDemoWindow(&show);
	if (openTelemetrics)
		drawTelemetry();
}
template <typename T>
T max(T a, T b)
{
	if (a < b)
		return b;
	return a;
}


void ImGuiLayer::onUpdate()
{
	
	updateTelemetry();
	
	
}

void ImGuiLayer::updateTelemetry()
{
	if (App::get().getInput().isKeyFreshlyPressed(GLFW_KEY_D))
		if (App::get().getInput().isKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
			openTelemetrics = !openTelemetrics;
			if (openTelemetrics)
				showTelemetrics = true;
		}
}


void ImGuiLayer::drawTelemetry()
{

	if (!ImGui::Begin("Telemetrics", &showTelemetrics,ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}
	if (!showTelemetrics) {
		openTelemetrics = false;
	}
	//todo following lines should be called by onupdate()
	if (ImGui::TreeNode("Tick")) {
		ImGui::PlotVar("Tick Millis", App::get().getTickMillis(),true, maxValResetDelay);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Render")) {
		ImGui::PlotVar("Render Millis", App::get().getRenderMillis(),true, maxValResetDelay);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("FPS")) {
		ImGui::PlotVar("FPS", App::get().getFPS(),false, maxValResetDelay);
		ImGui::PlotVar("Updates per Frame", App::get().getUpdatesPerFrame());
		ImGui::TreePop();
	}
	ImGui::Separator();
	ImGui::InputInt("Max val reset delay (ticks)", &maxValResetDelay);
	ImGui::Separator();
	ImGui::Value("Scheduled Tasks: ", ND_SCHED.size());
	ImGui::End();
}


