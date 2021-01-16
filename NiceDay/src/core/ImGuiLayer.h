#pragma once
#include "layer/Layer.h"
#include "core/NBT.h"
//renders specified texture as a imgui window

class NBT;
class Texture;
class FrameBuffer;

#define REGISTER_IMGUI_WIN(name,boolPtr) registerImGuiWinFunc(name,boolPtr)
void registerImGuiWinFunc(std::string_view v, bool* p);

enum class ImGuiLayout
{
	DEFAULT,
	SCREEN,
	CUSTOM0,
	NONE
};
constexpr const char* getLayoutName(ImGuiLayout type)
{
	switch (type) {
	case ImGuiLayout::DEFAULT: return "Default";
	case ImGuiLayout::SCREEN: return "Fullscreen";
	case ImGuiLayout::CUSTOM0: return "Custom0";
	}
	return "NOne";
}
inline ImGuiLayout getLayoutFromName(const char* type)
{
	if (!strcmp(type, getLayoutName(ImGuiLayout::DEFAULT)))
		return ImGuiLayout::DEFAULT;
	if (!strcmp(type, getLayoutName(ImGuiLayout::CUSTOM0)))
		return ImGuiLayout::CUSTOM0;
	if (!strcmp(type, getLayoutName(ImGuiLayout::SCREEN)))
		return ImGuiLayout::SCREEN;
	return ImGuiLayout::NONE;
}

class ImGuiLayer : public Layer
{
private:
	struct View
	{
		bool opened;
		std::string name;
		const Texture* texture;
		bool refreshed;
		//owner of texture (needs to call delete)
		bool owner;
	};
	std::vector<View> m_views;
	struct ImGuiWin
	{
		std::string name;
		bool* opened;
	};
	std::vector<ImGuiWin> m_wins;

	NBT m_wins_past;

	ImGuiLayout m_layout_type=ImGuiLayout::DEFAULT;
	std::string m_iniConfigToLoad;
	bool m_freshLayoutChange = false;
	std::string m_iniConfigToSave;
	void renderViewWindows();
	FrameBuffer* m_copyFBO;
public:
	ImGuiLayer();
	~ImGuiLayer() = default;
	void onAttach()override;
	void onDetach()override;
	void begin();
	void end();
	void onImGuiRender() override;
	void onUpdate() override;
	void onEvent(Event& e) override;
	void updateTelemetry();
	void drawTelemetry();
	void renderBaseImGui();
	// do this in onAttach to register window in window list,
	// opened will be switched to show or to completely hide the window
	void registerWindow(std::string_view windowName, bool* opened);
	
	void setINILayoutConfiguration(ImGuiLayout type,bool resetToDefault=false)
	{
		if (m_layout_type == type&&!resetToDefault)
			return;
		m_iniConfigToSave = std::string(getLayoutName(m_layout_type)) + ".ini";
		m_layout_type = type;
		if(!resetToDefault)
			m_iniConfigToLoad = std::string(getLayoutName(m_layout_type))+".ini";
		
		m_freshLayoutChange = resetToDefault || !std::filesystem::exists(std::string(getLayoutName(m_layout_type)) + ".ini");
	}

	void updateViewAnimation();
	void animateView();

	//must be submitted each frame to show imgui window with the texture
	void renderView(const std::string& name, const Texture* t);
	// same as renderView but the texture is immediately copied to another texture, so the base can change after
	// !Note!:
	//		changes current fbo!
	void renderViewProxy(const std::string& name, const Texture* t);
	
	

	void drawGlobals();
	static bool drawNBT(const char* name, NBT& n);
	static void drawNBTConst(const char* name, const NBT& n);
};


