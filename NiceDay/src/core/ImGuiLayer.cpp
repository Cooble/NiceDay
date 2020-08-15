#include "ImGuiLayer.h"
#include "imgui.h"
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include "App.h"
#include "GLFW/glfw3.h"
#include "AppGlobals.h"
#include "imgui_utils.h"
#include "event/MessageEvent.h"
#include "imgui_internal.h"
#include "graphics/API/Texture.h"
#include "graphics/API/FrameBuffer.h"
#include "graphics/Effect.h"
#include "scene/GlobalAccess.h"
#include <entt/entt.hpp>
#include "FakeWindow.h"

static void setImGuiCinema4DStyle()
{
	auto& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.24f, 0.24f, 0.24f, 0.95f);
	colors[ImGuiCol_Border] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.94f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.39f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.65f, 0.65f, 0.65f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);





	style.FrameBorderSize = 1;
	style.FrameRounding = 6;
	style.PopupRounding = 4;
	style.TabRounding = 9;
	style.ColorButtonPosition = ImGuiDir_Left;
}
struct MovingBox
{
	int index;
	glm::vec2 srcPos;
	glm::vec2 targetPos;
	glm::vec2 speed;
	float scaleSpeed;
	float scale = 0;

};
static bool s_is_animaiting = false;
static std::vector<MovingBox> s_boxes;
static const int view_size = 256;


void registerImGuiWinFunc(std::string_view v, bool* p)
{
	if(App::get().getIO().enableIMGUI)
		App::get().getImGui()->registerWindow(v, p);
}

void ImGuiLayer::renderViewWindows()
{
	for (int i = m_views.size() - 1; i >= 0; --i)
	{
		auto& view = m_views[i];
		if (!view.refreshed) {//remove those which have not been submitted this frame

			if (view.owner)
				delete view.texture;
			m_views.erase(m_views.begin() + i);
			continue;
		}
		view.refreshed = false;

		if (!view.opened)
			continue;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		//ImGui::SetNextWindowDockID(dock_left_id, ImGuiCond_Once);
		if (s_is_animaiting)
		{
			auto& box = s_boxes[i];
			ImGui::SetNextWindowPos(ImVec2(box.srcPos.x, box.srcPos.y), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(box.scale * view_size, box.scale * view_size), ImGuiCond_Always);
		}
		else {
			ImGui::SetNextWindowSize({ (float)view_size,(float)view_size }, ImGuiCond_Once);
		}
		ImGui::Begin(view.name.c_str(), &view.opened, ImGuiWindowFlags_NoDecoration);
		ImGui::PopStyleVar(2);
		auto size = ImGui::GetWindowSize();

		ImGui::Image((void*)view.texture->getID(), size, { 0,1 }, { 1,0 });
		ImGui::End();
	}

}

ImGuiLayer::ImGuiLayer()
	: Layer("ImGuiLayer")
{
	m_copyFBO = FrameBuffer::create();

}

static bool showTelemetrics = false;
static bool showDemoWindow = false;

static bool firstRun = false;
void ImGuiLayer::onAttach()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	setImGuiCinema4DStyle();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	/*if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}*/


	App& app = App::get();
	auto* window = static_cast<GLFWwindow*>(app.getPhysicalWindow()->getWindow());

	// Setup Platform/Renderer bindings
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410");

	GlobalAccess::ui_icons.createAtlas("res/models/scene_icons", 64, 1/*, TextureAtlasUVFlags_CreateFile*/);
	ImGui::GetIO().IniFilename = nullptr;

	registerWindow("Telemetrics", &showTelemetrics);
	registerWindow("Demo", &showDemoWindow);
	registerWindow("NavBar", &dynamic_cast<FakeWindow*>(App::get().getWindow())->m_enableNavigationBar);
	
	m_iniConfigToLoad = std::string(getLayoutName(m_layout_type)) + ".ini";
	m_freshLayoutChange = !std::filesystem::exists(std::string(getLayoutName(m_layout_type)) + ".ini");
	m_wins_past = NBT();
	NBT::loadFromFile(m_iniConfigToLoad + ".nbt", m_wins_past);
	
	firstRun = true;
}

void ImGuiLayer::onDetach()
{
	ImGui::SaveIniSettingsToDisk((std::string(getLayoutName(m_layout_type)) + ".ini").c_str());
	m_wins_past = NBT();
	for (auto& win : m_wins)
		m_wins_past[win.name] = *win.opened;
	NBT::saveToFile(std::string(getLayoutName(m_layout_type)) + ".ini.nbt", m_wins_past);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();


}

void ImGuiLayer::begin()
{
	if(firstRun)
	{
		firstRun = false;
		for (auto& win : m_wins)
			if (m_wins_past.exists(win.name))
				*win.opened = m_wins_past[win.name];
	}
	if (!m_iniConfigToSave.empty()) {
		ImGui::SaveIniSettingsToDisk(m_iniConfigToSave.c_str());
		m_wins_past = NBT();
		if (!m_freshLayoutChange)
			for (auto& win : m_wins)
				m_wins_past[win.name] = *win.opened;
	
		NBT::saveToFile(m_iniConfigToSave + ".nbt", m_wins_past);
		m_iniConfigToSave = "";
	}
	if (!m_iniConfigToLoad.empty()) {//todo imgui node crash on load ini settings: (assert) because of ImGui::DockContextBuildNodesFromSettings should be called before.
		ImGui::LoadIniSettingsFromDisk(m_iniConfigToLoad.c_str());
		m_wins_past = NBT();
		NBT::loadFromFile(m_iniConfigToLoad + ".nbt", m_wins_past);
		m_iniConfigToLoad = "";
		for (auto& win : m_wins)
			if(m_wins_past.exists(win.name))
				*win.opened = m_wins_past[win.name];
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (App::get().getIO().enableSCENE)
		renderBaseImGui();
}

void ImGuiLayer::end()
{
	ImGuiIO& io = ImGui::GetIO();
	App& app = App::get();
	io.DisplaySize = ImVec2(app.getWindow()->getWidth(), app.getWindow()->getHeight());

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}

static int maxValResetDelay = 60 * 2;


void ImGuiLayer::onImGuiRender()
{
	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);
	if (showTelemetrics)
		drawTelemetry();

	renderViewWindows();

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
	updateViewAnimation();
}

void ImGuiLayer::onEvent(Event& e)
{

	auto& io = ImGui::GetIO();
	if ((
		io.WantCaptureKeyboard && (e.getEventCategories() & Event::EventCategory::Key)) || (
			io.WantCaptureMouse && (e.getEventCategories() & Event::EventCategory::Mouse)))
	{
		//exception when view imgui window is focused - dont consume event
		if (App::get().getIO().enableSCENE && App::get().getWindow()->isFocused()) {

		}
		else {

			e.handled = true;
			return;
		}
	}

}

void ImGuiLayer::renderBaseImGui()
{
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetWorkPos());
		ImGui::SetNextWindowSize(viewport->GetWorkSize());
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	static bool p_open = true;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("MyDockSpace", &p_open, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	static ImGuiID dock_right_id;
	static ImGuiID dock_left_id = 0;
	static ImGuiID dock_down_id = 0;
	static ImGuiID dock_up_id = 0;
	// DockSpace
	ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_AutoHideTabBar;
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	//ImGui::Load
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);

	if (m_freshLayoutChange) {
		m_freshLayoutChange = false;

		if (m_layout_type == ImGuiLayout::DEFAULT|| m_layout_type == ImGuiLayout::CUSTOM0) {
			ImGui::DockBuilderRemoveNodeChildNodes(dockspace_id);
			ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.8f, &dock_left_id, &dock_right_id);
			ImGui::DockBuilderSplitNode(dock_left_id, ImGuiDir_Down, 0.2f, &dock_down_id, &dock_up_id);
			ImGui::DockBuilderDockWindow("FakeWindow", dock_up_id);
			ImGui::DockBuilderDockWindow("Scene", dock_right_id);
			ImGui::DockBuilderDockWindow("MMBrowser", dock_down_id);
			ImGui::DockBuilderFinish(dockspace_id);
		}
		else if (m_layout_type == ImGuiLayout::SCREEN)
		{
			ImGui::DockBuilderRemoveNodeChildNodes(dockspace_id);
			ImGui::DockBuilderDockWindow("FakeWindow", dockspace_id);
			ImGui::DockBuilderFinish(dockspace_id);
		}
	}


	static ImGuiWindowFlags viewWindowFlags = 0;

	if (ImGui::BeginMenuBar())
	{
		
		if (ImGui::BeginMenu("View"))
		{
			for (auto& view : m_views)
				if (ImGui::MenuItem(view.name.c_str(), "", view.opened))	view.opened = !view.opened;
			ImGui::Separator();

			if (ImGui::MenuItem("Enable All", "", false)) {
				for (auto& view : m_views) view.opened = true;
			}
			if (ImGui::MenuItem("Disable All", "", false))
				for (auto& view : m_views) view.opened = false;


			ImGui::PushStyleColor(ImGuiCol_Text, { 0.3f,0.3f,0.3f,1.f });
			if (ImGui::MenuItem("Enable All (EasterEgg)", "", false)) {
				for (auto& view : m_views) view.opened = true;
				animateView();
			}
			ImGui::PopStyleColor();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			for (auto& win : m_wins)
				ImGui::MenuItem(win.name.c_str(), "", win.opened);
			ImGui::EndMenu();
		}
		ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 60);
		if (ImGui::BeginMenu("Layout"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows,
			// which we can't undo at the moment without finer window depth/z control.
			//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

			if (ImGui::MenuItem(getLayoutName(ImGuiLayout::DEFAULT), "", m_layout_type == ImGuiLayout::DEFAULT, m_layout_type != ImGuiLayout::DEFAULT))
				this->setINILayoutConfiguration(ImGuiLayout::DEFAULT);
			if (ImGui::MenuItem(getLayoutName(ImGuiLayout::SCREEN), "", m_layout_type == ImGuiLayout::SCREEN, m_layout_type != ImGuiLayout::SCREEN))
				this->setINILayoutConfiguration(ImGuiLayout::SCREEN);
			if (ImGui::MenuItem(getLayoutName(ImGuiLayout::CUSTOM0), "", m_layout_type == ImGuiLayout::CUSTOM0, m_layout_type != ImGuiLayout::CUSTOM0))
				this->setINILayoutConfiguration(ImGuiLayout::CUSTOM0);
			ImGui::Separator();
			if (ImGui::MenuItem("Reset current layout"))
				this->setINILayoutConfiguration(m_layout_type, true);
			ImGui::EndMenu();

		}

		ImGui::EndMenuBar();
	}

	ImGui::End();
}

void ImGuiLayer::registerWindow(std::string_view windowName, bool* opened)
{
	for (auto& m_win : m_wins)
		if (m_win.name == windowName)
			return;
	m_wins.push_back({ std::string(windowName), opened });

}


int current_index_idx;

void ImGuiLayer::animateView()
{
	current_index_idx = 0;
	const int padding = 25;
	const float speed = 0.08;
	s_is_animaiting = true;
	s_boxes.clear();
	auto realDim = App::get().getPhysicalWindow()->getDimensions();
	auto realPos = App::get().getPhysicalWindow()->getPos();

	int currentX = padding;
	int currentY = padding;

	for (int i = 0; i < m_views.size(); ++i)
	{
		auto from = realPos + glm::vec2(2, 2);
		auto to = realPos + glm::vec2(currentX, currentY);
		//auto sp = glm::normalize(to - from) * speed;
		auto sp = (to - from) * speed;
		s_boxes.push_back({ i,from,to,sp,speed,0.01 });
		currentX += view_size + padding;
		if (currentX + view_size >= realDim.x)
		{
			currentX = padding;
			currentY += view_size + padding;
		}


	}


}

void ImGuiLayer::updateViewAnimation()
{
	if (s_boxes.size() == 0)
		return;
	static int will_die_in = 0;
	if (will_die_in > 0)
	{
		if (--will_die_in == 0)
			s_is_animaiting = false;
	}
	if (s_is_animaiting && will_die_in == 0)
	{
		auto& box = s_boxes[current_index_idx];
		box.srcPos += box.speed;
		box.scale += box.scaleSpeed;
		if (box.index >= m_views.size()) {
			s_is_animaiting = false;
			return;
		}
		if (
			box.srcPos.x + box.speed.x > box.targetPos.x &&
			box.srcPos.y + box.speed.y > box.targetPos.y)
		{
			box.scale = 1;
			box.srcPos = box.targetPos;
			if (++current_index_idx == s_boxes.size())
			{
				will_die_in = 5;//done
			}
		}




	}
}

void ImGuiLayer::renderView(const std::string& name, const  Texture* t)
{
	if (!t)
		return;
	for (auto& view : m_views)
	{
		if (view.name == name)
		{
			view.texture = t;
			view.refreshed = true;
			return;
		}
	}
	m_views.push_back({ true,name,t,true,false });

}

void ImGuiLayer::renderViewProxy(const std::string& name, const Texture* t)
{
	if (!t)
		return;
	for (auto& view : m_views)
	{
		if (view.name == name)
		{
			view.refreshed = true;
			if (!view.opened)
				return;//no render if window not opened

			ASSERT(view.owner, "Change in ownership not permitted");

			if (t->getDimensions() != view.texture->getDimensions()) {
				delete view.texture;
				view.texture = Texture::create(TextureInfo().size(t->getWidth(), t->getHeight()));
			}
			m_copyFBO->bind();
			m_copyFBO->attachTexture(view.texture->getID(), 0);
			Gcon.setViewport(t->getWidth(), t->getHeight());
			m_copyFBO->clear(BuffBit::COLOR);
			Gcon.disableBlend();
			Gcon.enableDepthTest(false);
			Effect::render(t, m_copyFBO);
			m_copyFBO->unbind();

			return;
		}
	}

	auto  tex = Texture::create(TextureInfo().size(t->getWidth(), t->getHeight()));
	m_copyFBO->bind();
	m_copyFBO->attachTexture(tex->getID(), 0);
	Gcon.setViewport(t->getWidth(), t->getHeight());
	Gcon.disableBlend();
	Gcon.enableDepthTest(false);
	Effect::render(t, m_copyFBO);
	m_copyFBO->unbind();

	m_views.push_back({ true,name,tex,true,true });

}

void ImGuiLayer::updateTelemetry()
{
	if (App::get().getInput().isKeyFreshlyPressed(KeyCode::D))
		if (App::get().getInput().isKeyPressed(KeyCode::LEFT_CONTROL))
			showTelemetrics = !showTelemetrics;
}

void ImGuiLayer::drawTelemetry()
{
	if (!ImGui::Begin("Telemetrics", &showTelemetrics,
		ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
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
	/*static bool shouldOpen;
	if (ImGui::Checkbox("Soundlayer", &shouldOpen))
	{
		if (shouldOpen)
			App::get().fireEvent(MessageEvent("openSoundLayer"));
		else App::get().fireEvent(MessageEvent("closeSoundLayer"));
	}*/
	auto mouseLoc = App::get().getInput().getMouseLocation();
	ImGui::Text("Mouse[%d, %d]", (int)mouseLoc.x, (int)mouseLoc.y);
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
	const char* names[] = { "String", "Int", "Float", "Bool", "Uint", "Map", "Array", "Null" };
	const char* namess[] = { "S", "I", "F", "B", "U", "M", "A", "N" };

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
		//ImGui::IsAnyWindowFocused()
		//if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
		//todo imdoc
		if (ImGui::IsAnyWindowFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
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
		ImGui::PushStyleColor(ImGuiCol_Text, { 0, 1, 0, 1 });
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
		ImGui::PushStyleColor(ImGuiCol_Text, { 0, 0, 1, 1 });
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
					if (drawNBT(bloatString(number, 4).c_str(), n[i]))
					{
						n.arrays().erase(n.arrays().begin() + i);
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
		ImGui::PushStyleColor(ImGuiCol_Text, { 0, 1, 0, 1 });
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
		}
		else
			ImGui::PopStyleColor();
	}
	else if (n.isArray())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, { 0, 0, 1, 1 });
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
		}
		else
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
