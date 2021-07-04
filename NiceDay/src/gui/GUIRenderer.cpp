#include "ndpch.h"
#include "GUIRenderer.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/GContext.h"
#include "core/AppGlobals.h"

namespace nd {

constexpr glm::vec4 darkColor(30 / 255.f, 30 / 255.f, 30 / 255.f, 1);
constexpr glm::vec4 backColor(45 / 255.f, 45 / 255.f, 48 / 255.f, 1);
constexpr glm::vec4 backlightColor(51 / 255.f, 51 / 255.f, 54 / 255.f, 1);
constexpr glm::vec4 lightColor(62 / 255.f, 62 / 255.f, 64 / 255.f, 1);
constexpr glm::vec4 lighterColor(80 / 255.f, 80 / 255.f, 80 / 255.f, 1);
constexpr glm::vec4 brightColor(28 / 255.f, 151 / 255.f, 234 / 255.f, 1);

GUIRenderer::GUIRenderer(glm::vec2 windowSize)
{
	m_view_fbo = FrameBuffer::create(FrameBufferInfo(windowSize.x, windowSize.y, TextureFormat::RGBA));
}

void GUIRenderer::setScreenDimensions(int w, int h)
{
	if (w == 0 || h == 0)
		return;
	m_view_fbo->resize(w, h);
}

static glm::vec4 randColor()
{
	return glm::vec4(std::rand() % 256 / 256.f, std::rand() % 256 / 256.f, std::rand() % 256 / 256.f, 1);
}

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

	bool rects;
	AppGlobals::get().nbt.loadSet("showGUIRect", rects, false);
	if (rects)
	{
		incrementZ();
		renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, randColor());
	}
	incrementZ();

	if (e.isVisible && e.isEnabled && !rects)
		renderElement(renderer, e);
	if (e.type != GETYPE::View && e.isEnabled) //view will render children on its own
		for (auto& element : e.getChildren())
			renderElements(renderer, *element);
	m_stackPos -= e.pos;
}


void GUIRenderer::renderElement(BatchRenderer2D& renderer, GUIElement& e)
{
	switch (e.type)
	{
	case GETYPE::TextBox:
		renderTextBox(renderer, static_cast<GUITextBox&>(e));
		break;
	case GETYPE::TextArea:
		//renderTextArea(renderer, static_cast<GUITextArea&>(e));
		break;
	case GETYPE::Image:
		renderImage(renderer, static_cast<GUIImage&>(e));
		break;
	case GETYPE::Window:
		{
			auto& t = static_cast<GUIWindow&>(e);
			if (t.isEnabled)
				renderWindow(renderer, t);
		}
		break;
	case GETYPE::Text:
		renderText(renderer, static_cast<GUIText&>(e));
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
	case GETYPE::VSlider:
		renderVSlider(renderer, static_cast<GUIVSlider&>(e));
		break;
	case GETYPE::HSlider:
		renderHSlider(renderer, static_cast<GUIHSlider&>(e));
		break;
	case GETYPE::View:
		renderView(renderer, static_cast<GUIView&>(e));
		break;
	case GETYPE::Blank:
		renderBlank(renderer, static_cast<GUIBlank&>(e));
		break;
	default:
		if (e.color.a != 0)
			renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, e.color);
		break;
	}
}

void GUIRenderer::updateTextMeshIfNec(const std::string& val, FontMaterial* mat, TextMesh& mesh, bool& isDirty,
                                      int alignment)
{
	if (isDirty)
	{
		isDirty = false;
		auto valId = SID(val);
		if (mesh.id == valId) //same text no need to rebuild
			return;
		auto codePoints = SUtil::utf8toCodePoints(val);
		if (mesh.getMaxCharCount() < codePoints.size() + 1)
			mesh.resize(val.size() + 1);
		mesh.id = valId;
		TextBuilder::buildMesh({codePoints}, *mat->font, mesh, alignment);
	}
}

void GUIRenderer::renderButton(BatchRenderer2D& renderer, GUIButton& e)
{
	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height},
	                         e.isPressed() ? brightColor : (e.hasFocus() ? lightColor : darkColor));
}

void GUIRenderer::renderCheckBox(BatchRenderer2D& renderer, GUICheckBox& e)
{
	updateTextMeshIfNec(e.getValue() ? e.getTrueText() : e.getFalseText(), m_font_material, e.m_text_mesh, e.isDirty,
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
		                         m_stackPos.x + e.padding[GUI_LEFT] + e.getGraphicalValue() * width - sliderW / 2,
		                         m_stackPos.y + e.height / 2 - sliderH / 2, m_z_pos
	                         }, {sliderW, sliderH},
	                         brightColor);
}

void GUIRenderer::renderVSlider(BatchRenderer2D& renderer, GUIVSlider& e)
{
	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, lighterColor);
	incrementZ();
	renderer.submitColorQuad({m_stackPos.x + 1, m_stackPos.y + 1, m_z_pos}, {e.width - 2, e.height - 2}, lightColor);
	incrementZ();
	float y = e.getGraphicalValue() * (e.height - e.heightPadding() - e.sliderHeight);

	renderer.submitColorQuad({m_stackPos.x + e.padding[GUI_LEFT], m_stackPos.y + y + e.padding[GUI_BOTTOM], m_z_pos},
	                         {e.width - e.widthPadding(), e.sliderHeight}, darkColor);
}

void GUIRenderer::renderHSlider(BatchRenderer2D& renderer, GUIHSlider& e)
{
	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, lighterColor);
	incrementZ();
	renderer.submitColorQuad({m_stackPos.x + 1, m_stackPos.y + 1, m_z_pos}, {e.width - 2, e.height - 2}, lightColor);
	incrementZ();
	float x = e.getGraphicalValue() * (e.width - e.widthPadding() - e.sliderWidth);

	renderer.submitColorQuad({m_stackPos.x + e.padding[GUI_LEFT] + x, m_stackPos.y + e.padding[GUI_BOTTOM], m_z_pos},
	                         {e.sliderWidth, e.height - e.heightPadding()}, darkColor);
}

void GUIRenderer::renderText(BatchRenderer2D& renderer, GUIText& e)
{
	int align = 0;
	switch (e.alignment)
	{
	case GUIAlign::RIGHT:
		align = TextBuilder::ALIGN_RIGHT;
		break;

	case GUIAlign::LEFT:
		align = TextBuilder::ALIGN_LEFT;
		break;

	case GUIAlign::CENTER:
	default:
		align = TextBuilder::ALIGN_CENTER;
		break;
	}
	updateTextMeshIfNec(e.getText(), e.fontMaterial, e.textMesh, e.isDirty, align);

	//renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, darkColor);
	//incrementZ();
	glm::mat4 mat(1.f);

	float x = m_stackPos.x;
	float y = m_stackPos.y + (e.height - e.fontMaterial->font->lineHeight * e.textScale) / 2;
	switch (e.alignment)
	{
	case GUIAlign::RIGHT:
		x += e.width - e.padding[GUI_RIGHT];
		break;

	case GUIAlign::LEFT:
		x += e.padding[GUI_LEFT];
		break;

	case GUIAlign::CENTER:
	default:
		x += e.padding[GUI_LEFT] + (e.width - e.widthPadding()) / 2;
		break;
	}
	mat = glm::translate(mat, {x, y, m_z_pos});

	mat = glm::scale(mat, {e.textScale, e.textScale, 1});

	renderer.push(mat);
	renderer.submitText(e.textMesh, e.fontMaterial);
	renderer.pop();
}

void GUIRenderer::renderTextBox(BatchRenderer2D& renderer, GUITextBox& e)
{
	CursorProp prop;
	prop.cursorCharacter = e.cursorChar;
	prop.cursorPos = e.cursorPos;
	bool wasdirty = e.isDirty;

	float trueWidth = e.width - e.widthPadding();
	if (e.isDirty)
	{
		e.cursorBlink = 0;
		e.isDirty = false;

		int textWidth = e.fontMaterial->font->getTextWidth(e.getValue().substr(0, e.cursorPos));
		int neededOffset = textWidth + e.textClipOffset;

		if (neededOffset > trueWidth)
		{
			e.textClipOffset -= neededOffset - trueWidth + 20;
		}
		else if (neededOffset < 0)
		{
			e.textClipOffset += -neededOffset + 20;
		}

		auto utf = SUtil::utf8toCodePoints(e.getValue());
		e.textMesh.reserve(utf.size());
		TextBuilder::buildMesh({utf}, *e.fontMaterial->font, e.textMesh, TextBuilder::ALIGN_LEFT,
		                       {-e.textClipOffset, -1000, -e.textClipOffset + trueWidth, 2000}, &prop);
	}

	if (wasdirty)
	{
		e.prop = prop;
		//change cursor pos
		e.cursorMesh.setChar(0,
		                     prop.positions.x, prop.positions.y, prop.positions.z, prop.positions.w, 0xffffff00, 0x000000,
		                     e.fontMaterial->font->getChar(e.cursorChar));
		e.cursorMesh.currentCharCount = 1;
	}

	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height},
	                         (e.hasFocus() || e.hasTotalFocus()) ? brightColor : lighterColor);
	incrementZ();
	renderer.submitColorQuad({m_stackPos.x + 1, m_stackPos.y + 1, m_z_pos}, {e.width - 2, e.height - 2},
	                         (e.hasFocus() || e.hasTotalFocus()) ? lightColor : backlightColor);
	incrementZ();
	renderer.push(glm::translate(glm::mat4(1.0), {
		                             m_stackPos.x + e.padding[GUI_LEFT] + e.textClipOffset,
		                             m_stackPos.y + (e.height - e.fontMaterial->font->lineHeight) / 2,
		                             m_z_pos
	                             }));
	renderer.submitText(e.textMesh, e.fontMaterial);


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
			                             m_stackPos.x + e.padding[GUI_LEFT] + e.textClipOffset,
			                             m_stackPos.y + (e.height - e.fontMaterial->font->lineHeight) / 2,
			                             m_z_pos
		                             }));
		renderer.submitText(e.cursorMesh, e.fontMaterial);
		renderer.pop();
	}
}

void GUIRenderer::renderWindow(BatchRenderer2D& renderer, GUIWindow& e)
{
	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, darkColor);
	incrementZ();
	renderer.submitColorQuad({m_stackPos.x + 3, m_stackPos.y + 3, m_z_pos}, {e.width - 6, e.height - 6}, backColor);
}

void GUIRenderer::renderView(BatchRenderer2D& renderer, GUIView& e)
{
	//todo make texture list cause i cannot use that texture multiple times when we have batch renderer
	//todo -> only one view per GuiRenderer is possible
	renderer.flush();
	m_view_fbo->bind();
	m_view_fbo->clear(BuffBit::COLOR, e.getInside()->color);
	renderer.begin(m_view_fbo);
	for (auto child : e.getChildren())
		renderElements(renderer, *child);
	renderer.flush();
	Renderer::getDefaultFBO()->bind();
	renderer.begin(Renderer::getDefaultFBO());

	auto dim = m_view_fbo->getSize();
	auto uv = UVQuad(
		{
			(m_stackPos.x + e.padding[GUI_LEFT]) / dim.x,
			(m_stackPos.y + e.padding[GUI_BOTTOM]) / dim.y
		},
		{
			(e.width - e.padding[GUI_LEFT] - e.padding[GUI_RIGHT]) / dim.x,
			(e.height - e.padding[GUI_TOP] - e.padding[GUI_BOTTOM]) / dim.y
		});
	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, brightColor);
	incrementZ();
	renderer.submitTextureQuad(
		{m_stackPos.x + e.padding[GUI_LEFT], m_stackPos.y + e.padding[GUI_BOTTOM], m_z_pos},
		{e.width - e.padding[GUI_LEFT] - e.padding[GUI_RIGHT], e.height - e.padding[GUI_TOP] - e.padding[GUI_BOTTOM]},
		uv, m_view_fbo->getAttachment(0));
}

void GUIRenderer::renderBlank(BatchRenderer2D& renderer, GUIBlank& e)
{
	if (e.color.a != 0)
		renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, e.color);
}

void GUIRenderer::renderImage(BatchRenderer2D& renderer, GUIImage& e)
{
	if (e.image)
		renderer.submitTextureQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, e.image->getUV(),
		                           e.image->getTexture());
}
}
