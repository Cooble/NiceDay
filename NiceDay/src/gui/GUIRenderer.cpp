#include "ndpch.h"
#include "GUIRenderer.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/GContext.h"

constexpr glm::vec4 darkColor(30 / 255.f, 30 / 255.f, 30 / 255.f, 1);
constexpr glm::vec4 backColor(45 / 255.f, 45 / 255.f, 48 / 255.f, 1);
constexpr glm::vec4 backlightColor(51 / 255.f, 51 / 255.f, 54 / 255.f, 1);
constexpr glm::vec4 lightColor(62 / 255.f, 62 / 255.f, 64 / 255.f, 1);
constexpr glm::vec4 lighterColor(80 / 255.f, 80 / 255.f, 80 / 255.f, 1);
constexpr glm::vec4 brightColor(28 / 255.f, 151 / 255.f, 234 / 255.f, 1);

void GUIRenderer::render(BatchRenderer2D& renderer)
{
	Gcon.enableDepthTest(true);
	m_stackPos = {0, 0};
	m_z_pos = 0;

	for (auto& window : m_context->getWindows())
		renderElements(renderer, *window);
}

void GUIRenderer::renderElements(BatchRenderer2D& renderer, GUIElement& e)
{
	m_stackPos += e.pos;
	if (e.isDisplayed())
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
	case GETYPE::TextBox:
		renderTextBox(renderer, static_cast<GUITextBox&>(e));
		break;
	case GETYPE::Image:
		renderImage(renderer, static_cast<GUIImage&>(e));
		break;
	case GETYPE::Window:
		renderWindow(renderer, static_cast<GUIWindow&>(e));
		break;
	case GETYPE::Label:
		renderLabel(renderer, static_cast<GUILabel&>(e));
		break;
	case GETYPE::Button:
		renderButton(renderer, static_cast<GUIButton&>(e));
		break;
	case GETYPE::CheckBox:
		renderCheckBox(renderer, static_cast<GUICheckBox&>(e));
		break;
	case GETYPE::Slider:
		renderSlider(renderer, static_cast<GUISlider&>(e));
		break;
	default:
		glm::vec4 color;

		if (e.clas == "white")
			color = {1, 1, 1, 1};
		else if (e.clas == "black")
			color = {0, 0, 0, 1};
		else
			color = {0, 0, 1, 1};
		renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, color);
	}
}

void GUIRenderer::updateTextMeshIfNec(const std::string& val, TextMesh& mesh, bool& isDirty, int alignment)
{
	if (isDirty)
	{
		isDirty = false;
		if (mesh.getMaxCharCount() < val.size() + 1)
			mesh.resize(val.size() + 1);
		TextBuilder::buildMesh({val}, *m_font_material->font, mesh, alignment);
	}
}

void GUIRenderer::updateTextMeshIfNec(const std::string& val, TextMesh& mesh, bool& isDirty, int alignment,
                                      glm::vec<4, int> clip, CursorProp* cursor)
{
	if (isDirty)
	{
		isDirty = false;
		if (mesh.getMaxCharCount() < val.size() + 1)
			mesh.resize(val.size() + 1);
		TextBuilder::buildMesh({val}, *m_font_material->font, mesh, alignment, clip, cursor);
	}
}

void GUIRenderer::renderButton(BatchRenderer2D& renderer, GUIButton& e)
{
	updateTextMeshIfNec(e.getText(), e.m_text_mesh, e.is_dirty, TextBuilder::ALIGN_CENTER);


	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height},
	                         e.isPressed() ? brightColor : (e.hasFocus() ? lightColor : darkColor));
	incrementZ();
	renderer.push(glm::translate(glm::mat4(1.0), {
		                             m_stackPos.x + e.width / 2,
		                             m_stackPos.y + (e.height - m_font_material->font->lineHeight) / 2, m_z_pos
	                             }));
	renderer.submitText(e.m_text_mesh, m_font_material);
	renderer.pop();
}

void GUIRenderer::renderCheckBox(BatchRenderer2D& renderer, GUICheckBox& e)
{
	updateTextMeshIfNec(e.getValue() ? e.getTrueText() : e.getFalseText(), e.m_text_mesh, e.is_dirty,
	                    TextBuilder::ALIGN_LEFT);


	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, darkColor);
	incrementZ();
	if (e.getValue())
		renderer.submitTextureQuad({m_stackPos.x + 5, m_stackPos.y + 5, m_z_pos}, {32, 32},
		                           e.spriteTrue->getUV(), e.spriteTrue->getTexture());
	else
		renderer.submitTextureQuad({m_stackPos.x + 5, m_stackPos.y + 5, m_z_pos}, {32, 32},
		                           e.spriteFalse->getUV(), e.spriteFalse->getTexture());
	incrementZ();
	renderer.push(glm::translate(glm::mat4(1.0), {
		                             m_stackPos.x + 42,
		                             m_stackPos.y + (e.height - m_font_material->font->lineHeight) / 2,
		                             m_z_pos
	                             }));
	renderer.submitText(e.m_text_mesh, m_font_material);
	renderer.pop();
}

void GUIRenderer::renderSlider(BatchRenderer2D& renderer, GUISlider& e)
{
	float width = e.width - e.padding[GUI_LEFT] - e.padding[GUI_RIGHT];
	constexpr float backLineHeight = 10;
	renderer.submitColorQuad(
		{m_stackPos.x + e.padding[GUI_LEFT], m_stackPos.y + e.height / 2 - backLineHeight / 2, m_z_pos},
		{width, backLineHeight},
		darkColor);

	incrementZ();
	constexpr float sliderW = 15;
	constexpr float sliderH = 30;
	renderer.submitColorQuad({
		                         m_stackPos.x + e.padding[GUI_LEFT] + e.getValue() * width - sliderW / 2,
		                         m_stackPos.y + e.height / 2 - sliderH / 2, m_z_pos
	                         }, {sliderW, sliderH},
	                         brightColor);
}

void GUIRenderer::renderLabel(BatchRenderer2D& renderer, GUILabel& e)
{
	updateTextMeshIfNec(e.getValue(), e.m_text_mesh, e.is_dirty, TextBuilder::ALIGN_LEFT);

	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, darkColor);
	incrementZ();
	renderer.push(glm::translate(glm::mat4(1.0), {
		                             m_stackPos.x, m_stackPos.y + (e.height - m_font_material->font->lineHeight) / 2,
		                             m_z_pos
	                             }));
	renderer.submitText(e.m_text_mesh, m_font_material);
	renderer.pop();
}

void GUIRenderer::renderTextBox(BatchRenderer2D& renderer, GUITextBox& e)
{
	CursorProp prop;
	prop.cursorCharacter = e.cursorChar;
	prop.cursorPos = e.cursorPos;
	bool wasdirty = e.is_dirty;

	float trueWidth = e.width - e.padding[GUI_LEFT] - e.padding[GUI_RIGHT];
	if (e.is_dirty)
	{
		e.cursorBlink = 0;
		e.is_dirty = false;

		int textWidth = m_font_material->font->getTextWidth(e.getValue().substr(0, e.cursorPos));
		int neededOffset = textWidth + e.textClipOffset;

		if(neededOffset>trueWidth)
		{
			e.textClipOffset -= neededOffset - trueWidth+20;
		}else if(neededOffset<0)
		{
			e.textClipOffset +=  -neededOffset+20;
		}
		
		
		if (e.m_text_mesh.getMaxCharCount() < e.getValue().size())
			e.m_text_mesh.resize(e.getValue().size());
		TextBuilder::buildMesh({ e.getValue() }, *m_font_material->font, e.m_text_mesh, TextBuilder::ALIGN_LEFT, {-e.textClipOffset,-1000,-e.textClipOffset+trueWidth,2000}, &prop);
	}
	
	if (wasdirty)
	{
		e.prop = prop;
		//change cursor pos
		e.m_cursorMesh.setChar(0,
		                       prop.positions.x, prop.positions.y, prop.positions.z, prop.positions.w,
		                       m_font_material->font->getChar(e.cursorChar));
		e.m_cursorMesh.currentCharCount = 1;
	}

	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height},
	                         (e.hasFocus() || e.hasTotalFocus()) ? brightColor : lighterColor);
	incrementZ();
	renderer.submitColorQuad({m_stackPos.x + 1, m_stackPos.y + 1, m_z_pos}, {e.width - 2, e.height - 2},
	                         (e.hasFocus() || e.hasTotalFocus()) ? lightColor : backlightColor);
	incrementZ();
	renderer.push(glm::translate(glm::mat4(1.0), {
		                             m_stackPos.x + e.padding[GUI_LEFT]+e.textClipOffset,
		                             m_stackPos.y + (e.height - m_font_material->font->lineHeight) / 2,
		                             m_z_pos
	                             }));
	renderer.submitText(e.m_text_mesh, m_font_material);


	renderer.pop();

	if (e.hasTotalFocus())
	{
		e.cursorBlink++;
		if (e.cursorBlink > 35)
			e.cursorBlink = -25;
		if (e.cursorBlink < 0)
			return;
		//show cursor only when fully focused
		incrementZ();
		renderer.push(glm::translate(glm::mat4(1.0), {
			                             m_stackPos.x + e.padding[GUI_LEFT]+e.textClipOffset,
			                             m_stackPos.y + (e.height - m_font_material->font->lineHeight) / 2,
			                             m_z_pos
		                             }));
		renderer.submitText(e.m_cursorMesh, m_font_material);
		renderer.pop();
	}
}

void GUIRenderer::renderWindow(BatchRenderer2D& renderer, GUIWindow& e)
{
	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, darkColor);
	incrementZ();
	renderer.submitColorQuad({m_stackPos.x + 3, m_stackPos.y + 3, m_z_pos}, {e.width - 6, e.height - 6}, backColor);
}

void GUIRenderer::renderImage(BatchRenderer2D& renderer, GUIImage& e)
{
	if (e.src)
		renderer.submitTextureQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, e.src->getUV(),
		                           e.src->getTexture());
}
