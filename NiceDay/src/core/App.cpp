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
#include "core/AppGlobals.h"
#include "lua/LuaLayer.h"
#include "event/ControlMap.h"
#include "event/KeyEvent.h"
#include "layer/SoundLayer.h"


#define BIND_EVENT_FN(x) std::bind(&App::x, &App::get(), std::placeholders::_1)


App* App::s_Instance = nullptr;


App::App(int width, int height, const std::string& title)
	: m_scheduler(200),
	  m_dbuff_stackalloc(1000000) //1MB
{
	m_thread_id = std::this_thread::get_id();
	ASSERT(s_Instance == nullptr, "Instance of game already exists!")
	s_Instance = this;
	init(width, height, title);
}

static void eventCallback(Event& e);

void App::init(int width, int height, const std::string& title)
{
	ControlMap::init();
	Log::init();
	m_Window = new Window(width, height, title);
	GContext::init(Renderer::getAPI());
	Effect::init();
	m_Window->setEventCallback(eventCallback);
	m_lua_layer = new LuaLayer();
	m_LayerStack.pushLayer(m_lua_layer);
	m_LayerStack.pushLayer(new SoundLayer());

	m_imgui_enable = true;
	if (m_imgui_enable)
	{
		m_ImGuiLayer = new ImGuiLayer();
		m_LayerStack.pushOverlay(m_ImGuiLayer);
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
	ND_TRACE("Game quitted");
	ND_PROFILE_END_SESSION();
}

void App::update()
{
	m_Window->pollEvents();
	m_Input.update();
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
	for (Layer* l : m_LayerStack)
		l->onRender();

	if (m_imgui_enable)
	{
		ND_PROFILE_SCOPE("imgui app render");
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
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
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
	m_imgui_consuming = ImGui::IsAnyItemActive() || ImGui::IsMouseHoveringAnyWindow();
	AppGlobals::get().nbt.save("imgui consumi", m_imgui_consuming);
}

static int maxValResetDelay = 60 * 2;
static bool openTelemetrics = false;
static bool showTelemetrics = false;
static int recordingScopeTicks = 0;

void ImGuiLayer::onImGuiRender()
{
	static bool show = true;
	ImGui::ShowDemoWindow(&show);
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
	if (recordingScopeTicks)
	{
		recordingScopeTicks--;
		if (recordingScopeTicks == 0)
		{
			ND_PROFILE_END_SESSION();
			ND_INFO("SCOPING done");
		}
	}
	updateTelemetry();
}

void ImGuiLayer::onEvent(Event& e)
{
	if (m_imgui_consuming && (
		e.getEventType() == Event::EventType::MousePress
		|| e.getEventType() == Event::EventType::KeyPress
		|| e.getEventType() == Event::EventType::KeyType
	))
		e.handled = true;
	else if (e.getEventType() == Event::EventType::KeyPress)
	{
		auto& m = dynamic_cast<KeyPressEvent&>(e);
		if (m.getKey() == GLFW_KEY_P && recordingScopeTicks == 0)
		{
			recordingScopeTicks = 60;
			ND_PROFILE_BEGIN_SESSION("test", "test.json");
			ND_INFO("SCOPING 1sec");
		}
	}
}

void ImGuiLayer::updateTelemetry()
{
	if (App::get().getInput().isKeyFreshlyPressed(GLFW_KEY_D))
		if (App::get().getInput().isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		{
			openTelemetrics = !openTelemetrics;
			if (openTelemetrics)
				showTelemetrics = true;
		}
}


void ImGuiLayer::drawTelemetry()
{
	if (!ImGui::Begin("Telemetrics", &showTelemetrics,
	                  ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}
	if (!showTelemetrics)
	{
		openTelemetrics = false;
	}
	//todo following lines should be called by onupdate()
	if (ImGui::TreeNode("Tick"))
	{
		ImGui::PlotVar("Tick Millis", App::get().getTickMillis(), true, maxValResetDelay);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Render"))
	{
		ImGui::PlotVar("Render Millis", App::get().getRenderMillis(), true, maxValResetDelay);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("FPS"))
	{
		ImGui::PlotVar("FPS", App::get().getFPS(), false, maxValResetDelay);
		ImGui::PlotVar("Updates per Frame", App::get().getUpdatesPerFrame());
		ImGui::TreePop();
	}
	ImGui::Separator();
	ImGui::InputInt("Max val reset delay (ticks)", &maxValResetDelay);
	ImGui::Separator();
	ImGui::Value("Scheduled Tasks: ", ND_SCHED.size());

	ImGui::Separator();
	drawGlobals();
	ImGui::End();
}

static Pool<char[128]> stringPool(500);

static int getNBTTypeIndex(const NBT& n)
{
	switch (n.types())
	{
	case NBT::T_STRING: return 0;
	case NBT::T_NUMBER_INT: return 1;
	case NBT::T_NUMBER_FLOAT: return 2;
	case NBT::T_BOOL: return 3;
	case NBT::T_NUMBER_UINT: return 4;
	case NBT::T_MAP: return 5;
	case NBT::T_ARRAY: return 6;
	case NBT::T_NULL: return 7;
	default: return -1;
	}
}

static auto hideLbl = "##hidelabel";

static void setNBTType(NBT& n, int index)
{
	switch (index)
	{
	case 0:
		n = "";
		break;
	case 1:
		n = (int64_t)n.toNumber();
		break;
	case 2:
		n = (float)n.toNumber();
		break;
	case 3:
		n = false;
		break;
	case 4:
		n = (uint64_t)n.toNumber();
		break;
	case 5:
		n = NBT();
		n["null1"] = NBT();
		break;
	case 6:
		n = NBT();
		n[0] = NBT();
		break;
	default:
		n = NBT();
		break;
	}
}

//return true on change
static bool typeSelector(NBT& n)
{
	int current_index = getNBTTypeIndex(n);
	//ImGui::PushItemWidth(50);
	ImGui::PushID(123);
	//bool change = ImGui::Combo(hideLbl, &current_index, "String\0Int\0Float\0Bool\0Uint\0Map\0Array\0Null\0\0");
	bool change = false;
	const char* names[] = {"String", "Int", "Float", "Bool", "Uint", "Map", "Array", "Null"};
	const char* namess[] = {"S", "I", "F", "B", "U", "M", "A", "N"};

	// Simple selection popup
	// (If you want to show the current selection inside the Button itself, you may want to build a string using the "###" operator to preserve a constant ID with a variable label)
	if (ImGui::SmallButton(namess[current_index]))
		ImGui::OpenPopup("my_select_popup");
	//ImGui::PopItemWidth();
	if (ImGui::BeginPopup("my_select_popup"))
	{
		for (int i = 0; i < IM_ARRAYSIZE(names); i++)
			if (ImGui::Selectable(names[i]))
			{
				current_index = i;
				setNBTType(n, current_index);
				change = true;
			}
		ImGui::EndPopup();
	}

	ImGui::PopID();
	return change;
}

static int maxKeyStringSize(const NBT& map)
{
	int out = 0;
	for (auto& value : map.maps())
		out = std::max((int)value.first.size(), out);
	return out;
}

static std::string bloatString(const std::string& s, int size)
{
	std::string out = s;
	out.reserve(size);
	while (out.size() < size)
		out += " ";
	return out;
}

static std::string newName;

//opens popup on previous item and sets newName possibly
//return true if renamed
static bool renameName(const char* name)
{
	bool rename = false;
	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(1))
	{
		ImGui::OpenPopup("my_select_popupo");
	}

	if (ImGui::BeginPopup("my_select_popupo", ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration))
	{
		char c[128]{};
		memcpy(c, name, strlen(name));
		if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("##heheheehj", c, 127, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			//ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
			newName = std::string(c);
			ImGui::CloseCurrentPopup();
			rename = true;
		}
		//else 
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}
	return rename;
}

//return true if should be removed
bool ImGuiLayer::drawNBT(const char* name, NBT& n)
{
	newName = "";

	if (!n.isContainer())
	{
		ImGui::Text(name);
		renameName(name);
		ImGui::SameLine();
	}
	if (n.isNumber())
		ImGui::PushItemWidth(50);
	if (n.isString())
	{
		char c[128]{};
		memcpy(c, n.c_str(), n.size());
		if (n.size() < 128)
		{
			if (ImGui::InputText(hideLbl, c, 128))
				n = std::string(c);
		}
		else
			ImGui::Text(n.string().c_str());
	}
	else if (n.isFloat())
	{
		float f = n;
		if (ImGui::InputFloat(hideLbl, &f, 0))
			n = f;
	}
	else if (n.isInt())
	{
		int f = n;
		if (ImGui::InputInt(hideLbl, &f, 0))
			n = (int64_t)f;
	}
	else if (n.isUInt())
	{
		int f = n;
		if (ImGui::InputInt(hideLbl, &f, 0))
			n = (uint64_t)f;
	}
	else if (n.isBool())
	{
		bool f = n;
		if (ImGui::Checkbox(hideLbl, &f))
			n = (bool)f;
	}
	else if (n.isMap())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, {0, 1, 0, 1});
		if (ImGui::TreeNode(name))
		{
			ImGui::PopStyleColor();
			renameName(name);
			int maxSize = maxKeyStringSize(n);
			ImGui::SameLine();
			if (!typeSelector(n))
			{
				ImGui::SameLine();
				if (ImGui::SmallButton("-"))
				{
					ImGui::TreePop();
					return true;
				}
				ImGui::Separator();
				for (auto& map : n.maps())
				{
					ImGui::PushID(map.first.c_str());
					if (drawNBT(bloatString(map.first, maxSize).c_str(), map.second))
					{
						n.maps().erase(n.maps().find(map.first));
						ImGui::PopID();
						break;
					}
					if (!newName.empty())
					{
						n.maps()[newName] = std::move(map.second);
						n.maps().erase(n.maps().find(map.first));
						ImGui::PopID();
						newName = "";
						break;
					}
					ImGui::PopID();
				}
				if (ImGui::SmallButton("+"))
				{
					int nulls = 0;
					while (n.exists("null" + std::to_string(++nulls)));
					std::string s = "null" + std::to_string(nulls);
					n[s] = NBT();
				}
			}
			ImGui::TreePop();
			return false;
		}
		ImGui::PopStyleColor();
		renameName(name);
	}
	else if (n.isArray())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, {0, 0, 1, 1});
		if (ImGui::TreeNode(name))
		{
			ImGui::PopStyleColor();
			renameName(name);
			ImGui::SameLine();
			if (!typeSelector(n))
			{
				ImGui::SameLine();
				if (ImGui::SmallButton("-"))
				{
					ImGui::TreePop();
					return true;
				}
				ImGui::Separator();
				for (int i = 0; i < n.size(); ++i)
				{
					std::string number = "[" + std::to_string(i) + "]";
					ImGui::PushID(i);
					if(drawNBT(bloatString(number, 4).c_str(), n[i]))
					{
						n.arrays().erase(n.arrays().begin()+i);
						ImGui::PopID();
						break;
					}
					ImGui::PopID();
				}
				if (ImGui::SmallButton("+"))
				{
					n[n.size()] = NBT();
				}
			}
			ImGui::TreePop();
			return false;
		}
		ImGui::PopStyleColor();
		renameName(name);
	}
	else
	{
		ImGui::Text("null");
	}
	if (n.isNumber())
		ImGui::PopItemWidth();
	if (!n.isContainer())
	{
		ImGui::SameLine();
		typeSelector(n);
		ImGui::SameLine();
		return ImGui::SmallButton("-");
	}
	return false;
}

void ImGuiLayer::drawNBTConst(const char* name, const NBT& n)
{
	newName = "";

	if (!n.isContainer())
	{
		ImGui::Text(name);
		ImGui::SameLine();
	}
	if (n.isNumber())
		ImGui::PushItemWidth(50);
	if (n.isString())
	{
		ImGui::Text(n.string().c_str());
	}
	else if (n.isFloat())
	{
		ImGui::Value("", (float)n);
	}
	else if (n.isInt())
	{
		ImGui::Value("", (int32_t)n);
	}
	else if (n.isUInt())
	{
		ImGui::Value("", (uint32_t)n);
	}
	else if (n.isBool())
	{
		ImGui::Value("", (bool)n);
	}
	else if (n.isMap())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, {0, 1, 0, 1});
		if (ImGui::TreeNode(name))
		{
			ImGui::PopStyleColor();
			int maxSize = maxKeyStringSize(n);
			ImGui::SameLine();
			ImGui::Separator();
			for (auto& map : n.maps())
			{
				ImGui::PushID(map.first.c_str());
				drawNBTConst(bloatString(map.first, maxSize).c_str(), map.second);
				ImGui::PopID();
			}
			ImGui::TreePop();
		}else
		ImGui::PopStyleColor();
	}
	else if (n.isArray())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, {0, 0, 1, 1});
		if (ImGui::TreeNode(name))
		{
			ImGui::PopStyleColor();
			ImGui::SameLine();
				ImGui::Separator();
				for (int i = 0; i < n.size(); ++i)
				{
					std::string number = "[" + std::to_string(i) + "]";
					ImGui::PushID(i);
					drawNBTConst(bloatString(number, 4).c_str(), n[i]);
					ImGui::PopID();
				}
			
			ImGui::TreePop();
		}else
		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::Text("null");
	}
	if (n.isNumber())
		ImGui::PopItemWidth();
}

void ImGuiLayer::drawGlobals()
{
	drawNBT("Globals", AppGlobals::get().nbt);
	drawNBTConst("GlobalsConst", AppGlobals::get().nbt);
}
