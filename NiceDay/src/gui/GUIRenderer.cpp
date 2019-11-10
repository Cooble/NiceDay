#include "ndpch.h"
#include "GUIRenderer.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/GContext.h"

constexpr glm::vec4 backColor(45 / 255.f, 45 / 255.f, 48 / 255.f, 1);
constexpr glm::vec4 darkColor(30 / 255.f, 30 / 255.f, 30 / 255.f, 1);
constexpr glm::vec4 lightColor(62 / 255.f, 62 / 255.f, 64 / 255.f, 1);
constexpr glm::vec4 brightColor(28 / 255.f, 151 / 255.f, 234 / 255.f, 1);
void GUIRenderer::render(BatchRenderer2D& renderer)
{
	Gcon.enableDepthTest(true);
	m_stackPos = { 0,0};
	m_z_pos = 0;
	
	for (auto& window : m_context->getWindows())
		renderElements(renderer, *window);
}

void GUIRenderer::renderElements(BatchRenderer2D& renderer, GUIElement& e)
{
	m_stackPos += e.pos;
	if(e.isDisplayed())
		renderElement(renderer, e);
	for (auto& element : e.getChildren())
		renderElements(renderer, *element);
	m_stackPos -= e.pos;
}

void GUIRenderer::renderElement(BatchRenderer2D& renderer, GUIElement& e)
{
	incrementZ();
	switch (e.type)
	{
	case GETYPE::Window:
		renderWindow(renderer, static_cast<GUIWindow&>(e));
		break;
	case GETYPE::Label:
		renderLabel(renderer, static_cast<GUILabel&>(e));
		break;
	case GETYPE::Button:
		renderButton(renderer, static_cast<GUIButton&>(e));
		break;
	default:
		glm::vec4 color;

		if (e.clas == "white")
			color = { 1, 1, 1, 1 };
		else if (e.clas == "black")
			color = { 0, 0, 0, 1 };
		else
			color = { 0, 0, 1, 1 };
		renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos }, {e.width, e.height}, color);
	}
}

void GUIRenderer::updateTextMeshIfNec(const std::string& val, TextMesh& mesh, bool& isDirty,int alignment)
{
	if (isDirty) {
		isDirty = false;
		if (mesh.getMaxCharCount() < val.size())
			mesh.resize(val.size());
		TextBuilder::buildMesh({ val }, *m_font_material->font, mesh, alignment);
	}
}

void GUIRenderer::renderButton(BatchRenderer2D& renderer, GUIButton& e)
{
	updateTextMeshIfNec(e.getValue(), e.m_text_mesh, e.is_dirty, TextBuilder::ALIGN_CENTER);

	
	renderer.submitColorQuad({ m_stackPos.x, m_stackPos.y, m_z_pos }, { e.width, e.height }, e.isPressed()?brightColor:(e.hasFocus()?lightColor:darkColor));
	incrementZ();
	renderer.push(glm::translate(glm::mat4(1.0), { m_stackPos.x + e.width / 2,m_stackPos.y + (e.height - m_font_material->font->lineHeight) / 2,m_z_pos}));
	renderer.submitText(e.m_text_mesh, m_font_material);
	renderer.pop();

}

void GUIRenderer::renderLabel(BatchRenderer2D& renderer, GUILabel& e)
{
	updateTextMeshIfNec(e.getValue(), e.m_text_mesh, e.is_dirty, TextBuilder::ALIGN_LEFT);

	renderer.submitColorQuad({ m_stackPos.x, m_stackPos.y, m_z_pos }, { e.width, e.height }, darkColor);
	incrementZ();
	renderer.push(glm::translate(glm::mat4(1.0), { m_stackPos.x,m_stackPos.y + (e.height - m_font_material->font->lineHeight) / 2,m_z_pos }));
	renderer.submitText(e.m_text_mesh, m_font_material);
	renderer.pop();
}

void GUIRenderer::renderWindow(BatchRenderer2D& renderer, GUIWindow& e)
{
	renderer.submitColorQuad({ m_stackPos.x, m_stackPos.y, m_z_pos }, { e.width, e.height }, darkColor);
	incrementZ();
	renderer.submitColorQuad({ m_stackPos.x+3, m_stackPos.y+3, m_z_pos }, { e.width-6, e.height-6 }, backColor);
}
