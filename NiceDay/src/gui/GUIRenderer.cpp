#include "ndpch.h"
#include "GUIRenderer.h"
#include "graphics/BatchRenderer2D.h"
#include "graphics/GContext.h"
#include "App.h"
#include "AppGlobals.h"

constexpr glm::vec4 darkColor(30 / 255.f, 30 / 255.f, 30 / 255.f, 1);
constexpr glm::vec4 backColor(45 / 255.f, 45 / 255.f, 48 / 255.f, 1);
constexpr glm::vec4 backlightColor(51 / 255.f, 51 / 255.f, 54 / 255.f, 1);
constexpr glm::vec4 lightColor(62 / 255.f, 62 / 255.f, 64 / 255.f, 1);
constexpr glm::vec4 lighterColor(80 / 255.f, 80 / 255.f, 80 / 255.f, 1);
constexpr glm::vec4 brightColor(28 / 255.f, 151 / 255.f, 234 / 255.f, 1);

GUIRenderer::GUIRenderer(glm::vec2 windowSize)
{
	m_view_fbo = FrameBuffer::create();
	setScreenDimensions(windowSize.x, windowSize.y);

}

void GUIRenderer::setScreenDimensions(int w, int h)
{
	if (w == 0 || h == 0)
		return;
	if(m_view_texture)
		delete m_view_texture;
	m_view_texture = Texture::create(TextureInfo().size(w, h).format(TextureFormat::RGB));
	m_view_fbo->bind();
	m_view_fbo->attachTexture(m_view_texture->getID(), 0);
	m_view_fbo->unbind();
}

void GUIRenderer::render(BatchRenderer2D& renderer)
{
	TimerStaper s("");

	Gcon.enableDepthTest(true);
	m_stackPos = {0, 0};
	m_z_pos = 0;

	for (auto& window : m_context->getWindows())
		renderElements(renderer, *window);

	ND_GLOBAL_LOG("GUI render ms", (float)(s.getUS()/1000.f));
}

void GUIRenderer::renderElements(BatchRenderer2D& renderer, GUIElement& e)
{
	m_stackPos += e.pos;

	if(e.isDisplayed())
		renderElement(renderer, e);
	if (e.type != GETYPE::View) //view will render children on its own
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
	case GETYPE::TextArea:
		//renderTextArea(renderer, static_cast<GUITextArea&>(e));
		break;
	case GETYPE::Image:
		renderImage(renderer, static_cast<GUIImage&>(e));
		break;
	case GETYPE::Window:
		renderWindow(renderer, static_cast<GUIWindow&>(e));
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
	case GETYPE::View:
		renderView(renderer, static_cast<GUIView&>(e));
		break;
	default:
		if (e.color.a != 0)
			renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, e.color);
		break;
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
	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height},
	                         e.isPressed() ? brightColor : (e.hasFocus() ? lightColor : darkColor));
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

void GUIRenderer::renderVSlider(BatchRenderer2D& renderer, GUIVSlider& e)
{
	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, lighterColor);
	incrementZ();
	renderer.submitColorQuad({m_stackPos.x + 1, m_stackPos.y + 1, m_z_pos}, {e.width - 2, e.height - 2}, lightColor);
	incrementZ();
	float y = e.getValue() * (e.height - e.padding[GUI_TOP] - e.padding[GUI_BOTTOM] - e.sliderHeight);

	renderer.submitColorQuad({m_stackPos.x + e.padding[GUI_LEFT], m_stackPos.y + y + e.padding[GUI_BOTTOM], m_z_pos},
	                         {e.width - e.widthPadding(), e.sliderHeight}, darkColor);
}

void GUIRenderer::renderText(BatchRenderer2D& renderer, GUIText& e)
{
	int align = 0;
	switch (e.getAlignment())
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
	updateTextMeshIfNec(e.getText(), e.textMesh, e.is_dirty, align);

	//renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, darkColor);
	//incrementZ();
	glm::mat4 mat(1.f);

	float x = m_stackPos.x;
	float y = m_stackPos.y+ (e.height - m_font_material->font->lineHeight * e.textScale) / 2;
	switch (e.getAlignment())
	{
	case GUIAlign::RIGHT:
		x += e.width - e.padding[GUI_RIGHT];
		break;

	case GUIAlign::LEFT:
		x += e.padding[GUI_LEFT];
		break;

	case GUIAlign::CENTER:
	default:
		x += e.padding[GUI_LEFT]+(e.width - e.widthPadding())/2;
		break;
	}
	mat = glm::translate(mat, {x, y, m_z_pos});

	mat = glm::scale(mat, {e.textScale, e.textScale, 1});

	renderer.push(mat);
	renderer.submitText(e.textMesh, m_font_material);
	renderer.pop();
}

void GUIRenderer::renderTextBox(BatchRenderer2D& renderer, GUITextBox& e)
{
	CursorProp prop;
	prop.cursorCharacter = e.cursorChar;
	prop.cursorPos = e.cursorPos;
	bool wasdirty = e.is_dirty;

	float trueWidth = e.width - e.widthPadding();
	if (e.is_dirty)
	{
		e.cursorBlink = 0;
		e.is_dirty = false;

		int textWidth = m_font_material->font->getTextWidth(e.getValue().substr(0, e.cursorPos));
		int neededOffset = textWidth + e.textClipOffset;

		if (neededOffset > trueWidth)
		{
			e.textClipOffset -= neededOffset - trueWidth + 20;
		}
		else if (neededOffset < 0)
		{
			e.textClipOffset += -neededOffset + 20;
		}


		e.textMesh.reserve(e.getValue().size());
		TextBuilder::buildMesh({e.getValue()}, *m_font_material->font, e.textMesh, TextBuilder::ALIGN_LEFT,
		                       {-e.textClipOffset, -1000, -e.textClipOffset + trueWidth, 2000}, &prop);
	}

	if (wasdirty)
	{
		e.prop = prop;
		//change cursor pos
		e.cursorMesh.setChar(0,
		                       prop.positions.x, prop.positions.y, prop.positions.z, prop.positions.w,
		                       m_font_material->font->getChar(e.cursorChar));
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
		                             m_stackPos.y + (e.height - m_font_material->font->lineHeight) / 2,
		                             m_z_pos
	                             }));
	renderer.submitText(e.textMesh, m_font_material);


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
			                             m_stackPos.y + (e.height - m_font_material->font->lineHeight) / 2,
			                             m_z_pos
		                             }));
		renderer.submitText(e.cursorMesh, m_font_material);
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
	renderer.begin();
	m_view_fbo->bind();
	Gcon.setClearColor(e.getInside()->color);
	Gcon.clear(BufferBit::COLOR_BUFFER_BIT);
	Gcon.clear(BufferBit::DEPTH_BUFFER_BIT);
	Gcon.setViewport(m_view_texture->getWidth(), m_view_texture->getHeight());
	for (auto child : e.getChildren())
		renderElements(renderer, *child);
	renderer.flush();
	renderer.begin();
	m_view_fbo->unbind();
	Gcon.setViewport(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());


	auto uv = UVQuad(
		{
			(m_stackPos.x + e.padding[GUI_LEFT]) / m_view_texture->getWidth(),
			(m_stackPos.y + e.padding[GUI_BOTTOM]) / m_view_texture->getHeight()
		},
		{
			(e.width - e.padding[GUI_LEFT] - e.padding[GUI_RIGHT]) / m_view_texture->getWidth(),
			(e.height - e.padding[GUI_TOP] - e.padding[GUI_BOTTOM]) / m_view_texture->getHeight()
		});
	renderer.submitColorQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, brightColor);
	incrementZ();
	renderer.submitTextureQuad(
		{m_stackPos.x + e.padding[GUI_LEFT], m_stackPos.y + e.padding[GUI_BOTTOM], m_z_pos},
		{e.width - e.padding[GUI_LEFT] - e.padding[GUI_RIGHT], e.height - e.padding[GUI_TOP] - e.padding[GUI_BOTTOM]},
		uv, m_view_texture);
}

void GUIRenderer::renderImage(BatchRenderer2D& renderer, GUIImage& e)
{
	if (e.src)
		renderer.submitTextureQuad({m_stackPos.x, m_stackPos.y, m_z_pos}, {e.width, e.height}, e.src->getUV(),
		                           e.src->getTexture());
}
