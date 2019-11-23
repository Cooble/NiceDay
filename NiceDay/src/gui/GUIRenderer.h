#pragma once
#include "GUIContext.h"
#include "graphics/FontMaterial.h"
#include "graphics/API/FrameBuffer.h"

class BatchRenderer2D;

constexpr float Z_DIR_STEP = 1 / 1000.f;
class GUIRenderer
{
private:
	GUIContext* m_context;
	glm::vec2 m_stackPos;
	float m_z_pos;
	bool is_view_render_mode;
	FrameBuffer* m_view_fbo;
	Texture* m_view_texture=nullptr;
	
	inline void incrementZ()
	{
		m_z_pos -= Z_DIR_STEP;
	}
	void updateTextMeshIfNec(const std::string& val, TextMesh& mesh, bool& isDirty, int alignment);
	void updateTextMeshIfNec(const std::string& val, TextMesh& mesh, bool& isDirty, int alignment, glm::vec<4, int> clip,CursorProp* prop);

	void renderButton(BatchRenderer2D& renderer, GUIButton& e);
	void renderCheckBox(BatchRenderer2D& renderer, GUICheckBox& e);
	void renderSlider(BatchRenderer2D& renderer, GUISlider& e);
	void renderVSlider(BatchRenderer2D& renderer, GUIVSlider& e);
	void renderText(BatchRenderer2D& renderer, GUIText& e);
	void renderTextBox(BatchRenderer2D& renderer, GUITextBox& e);
	void renderWindow(BatchRenderer2D& renderer, GUIWindow& e);

	void renderView(BatchRenderer2D& renderer, GUIView& e);

	void renderImage(BatchRenderer2D& renderer, GUIImage& e);
	void renderElement(BatchRenderer2D& renderer, GUIElement& e);
	void renderElements(BatchRenderer2D& renderer, GUIElement& e);
public:
	GUIRenderer(glm::vec2 windowSize = {1280,720});
	inline void setContext(GUIContext* context) { m_context = context; }
	FontMaterial* m_font_material;

	void setScreenDimensions(int w,int h);
	

	void render(BatchRenderer2D& renderer);
};

