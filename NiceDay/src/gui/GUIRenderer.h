#pragma once
#include "GUIContext.h"
#include "graphics/FontMaterial.h"

class BatchRenderer2D;

constexpr float Z_DIR_STEP = 1 / 1000.f;
class GUIRenderer
{
private:
	GUIContext* m_context;
	glm::vec2 m_stackPos;
	float m_z_pos;

	inline void incrementZ()
	{
		m_z_pos -= Z_DIR_STEP;
	}
	void updateTextMeshIfNec(const std::string& val, TextMesh& mesh, bool& isDirty, int alignment);
	void renderButton(BatchRenderer2D& renderer, GUIButton& e);
	void renderCheckBox(BatchRenderer2D& renderer, GUICheckBox& e);
	void renderSlider(BatchRenderer2D& renderer, GUISlider& e);
	void renderLabel(BatchRenderer2D& renderer, GUILabel& e);
	void renderTextBox(BatchRenderer2D& renderer, GUITextBox& e);
	void renderWindow(BatchRenderer2D& renderer, GUIWindow& btn);

	void renderImage(BatchRenderer2D& renderer, GUIImage& guiImage);
	void renderElement(BatchRenderer2D& renderer, GUIElement& e);
	void renderElements(BatchRenderer2D& renderer, GUIElement& e);
public:
	GUIRenderer() = default;
	inline void setContext(GUIContext* context) { m_context = context; }
	FontMaterial* m_font_material;

	void render(BatchRenderer2D& renderer);
};

