#pragma once
#include "layer/Layer.h"
//renders specified texture as a imgui window

class NBT;
class Texture;
class FrameBuffer;

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


