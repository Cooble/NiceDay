#pragma once
#include "GUIContext.h"
#include "graphics/FontMaterial.h"
#include "graphics/API/FrameBuffer.h"

class BatchRenderer2D;

constexpr float Z_DIR_STEP = 1 / 1000.f;
class GUIRenderer
{
protected:
	GUIContext* m_context;
	glm::vec2 m_stackPos;
	float m_z_pos;
	bool is_view_render_mode;
	FrameBuffer* m_view_fbo;
	
	void incrementZ()
	{
		m_z_pos -= Z_DIR_STEP;
	}

	virtual void updateTextMeshIfNec(const std::string& val, FontMaterial* mat, TextMesh& mesh, bool& isDirty, int alignment);

	virtual void renderButton(BatchRenderer2D& renderer, GUIButton& e);
	virtual void renderCheckBox(BatchRenderer2D& renderer, GUICheckBox& e);
	virtual void renderSlider(BatchRenderer2D& renderer, GUISlider& e);
	virtual void renderVSlider(BatchRenderer2D& renderer, GUIVSlider& e);
	virtual void renderHSlider(BatchRenderer2D& renderer, GUIHSlider& e);
	virtual void renderText(BatchRenderer2D& renderer, GUIText& e);
	virtual void renderTextBox(BatchRenderer2D& renderer, GUITextBox& e);
	virtual void renderWindow(BatchRenderer2D& renderer, GUIWindow& e);


	virtual void renderView(BatchRenderer2D& renderer, GUIView& e);
	virtual void renderBlank(BatchRenderer2D& renderer, GUIBlank& e);
	
	virtual void renderImage(BatchRenderer2D& renderer, GUIImage& e);
	virtual void renderElement(BatchRenderer2D& renderer, GUIElement& e);
	virtual void renderElements(BatchRenderer2D& renderer, GUIElement& e);
public:
	GUIRenderer(glm::vec2 windowSize = {1280,720});
	void setContext(GUIContext* context) { m_context = context; }
	FontMaterial* m_font_material;

	virtual void setScreenDimensions(int w,int h);
	

	virtual void render(BatchRenderer2D& renderer);
};



