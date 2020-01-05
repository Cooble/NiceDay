#include "GUICustomRenderer.h"
#include "graphics/BatchRenderer2D.h"
#include "core/AppGlobals.h"
#include "graphics/GContext.h"
#include "core/App.h"
#include "GUIItemContainer.h"

GUICustomRenderer::GUICustomRenderer(glm::vec2 windowSize)
:GUIRenderer(windowSize)
{
	m_small_font = FontMatLib::getMaterial("res/fonts/andrew.fnt");
	
	static SpriteSheetResource* res = new SpriteSheetResource(
		Texture::create(TextureInfo("res/images/gui_atlas.png").filterMode(TextureFilterMode::NEAREST)),
		8, 8);

	if (!m_corn_left_down.isEnabled())
	{
		m_corn_left_down = Sprite(res);
		m_corn_right_down = Sprite(res);
		m_corn_left_up = Sprite(res);
		m_corn_right_up = Sprite(res);
		m_left = Sprite(res);
		m_up = Sprite(res);
		m_right = Sprite(res);
		m_down = Sprite(res);

		m_corn_left_down.setSpriteIndex(0, 0, true);
		m_corn_right_down.setSpriteIndex(0, 0);
		m_corn_left_up.setSpriteIndex(0, 0, true, true);
		m_corn_right_up.setSpriteIndex(0, 0, false, true);

		m_left.setSpriteIndex(1, 0, true, false);
		m_up.setSpriteIndex(1, 0, true, false, true);
		m_right.setSpriteIndex(1, 0);
		m_down.setSpriteIndex(1, 0, false, true, true);
	}
}

void GUICustomRenderer::setItemAtlas(int atlasSize, Texture* t)
{
	m_item_atlas_size = atlasSize;
	m_item_atlas_bit = 1.f / m_item_atlas_size;
	m_item_texture = t;
}

void GUICustomRenderer::render(BatchRenderer2D& renderer)
{
	TimerStaper s("");
	GUIRenderer::render(renderer);
	ND_GLOBAL_LOG("GUI render ms", (float)(s.getUS() / 1000.f));

}

void GUICustomRenderer::renderRectangle(BatchRenderer2D& renderer, float x, float y, float width, float height)
{
	x = (int)x;
	y = (int)y;
	width = (int)width;
	height = (int)height;
	
	constexpr  float dim = 16;
	renderer.submitTextureQuad({ x,y,m_z_pos }, { dim,dim }, m_corn_left_down.getUV(), m_corn_left_down.getTexture());
	incrementZ();
	renderer.submitTextureQuad({ x + width - dim,y,m_z_pos }, { dim,dim }, m_corn_right_down.getUV(), m_corn_right_down.getTexture());
	incrementZ();
	renderer.submitTextureQuad({ x + width - dim,y + height - dim,m_z_pos }, { dim,dim }, m_corn_right_up.getUV(), m_corn_right_up.getTexture());
	incrementZ();
	renderer.submitTextureQuad({ x,y + height - dim,m_z_pos }, { dim,dim }, m_corn_left_up.getUV(), m_corn_left_up.getTexture());
	incrementZ();
	renderer.submitTextureQuad({ x + dim,y,m_z_pos }, { width - 2 * dim,dim }, m_down.getUV(), m_down.getTexture());
	incrementZ();
	renderer.submitTextureQuad({ x + dim,y + height - dim,m_z_pos }, { width - 2 * dim,dim }, m_up.getUV(), m_up.getTexture());
	incrementZ();
	renderer.submitTextureQuad({ x ,y + dim,m_z_pos }, { dim,height - 2 * dim }, m_left.getUV(), m_left.getTexture());
	incrementZ();
	renderer.submitTextureQuad({ x + width - dim ,y + dim,m_z_pos }, { dim,height - 2 * dim }, m_right.getUV(), m_right.getTexture());
	incrementZ();
	renderer.submitColorQuad({ x + dim,y + dim,m_z_pos }, { width - 2 * dim,height - 2 * dim }, guiCRColor);
}

void GUICustomRenderer::renderElement(BatchRenderer2D& renderer, GUIElement& e)
{
	if(e.type==GETYPE::ItemContainer)
	{
		renderItemContainer(renderer, static_cast<GUIItemContainer&>(e));
	}
	else
		GUIRenderer::renderElement(renderer, e);
}

void GUICustomRenderer::renderWindow(BatchRenderer2D& renderer, GUIWindow& e)
{
	renderRectangle(renderer, m_stackPos.x, m_stackPos.y, e.width, e.height);
}

void GUICustomRenderer::renderItemContainer(BatchRenderer2D& renderer, GUIItemContainer& e)
{
	float scale = e.slotScale;

	//background for slot
	if (e.isSlotRendered) {
		renderRectangle(renderer, m_stackPos.x-scale, m_stackPos.y- scale, e.width+ scale*2, e.height + scale*2);
		incrementZ();
	}

	//item
	auto stack = e.getItemStack();
	if (stack == nullptr)
		return;
	auto& item = stack->getItem();

	half_int txtOffset = item.getTextureOffset(*stack);
	
	renderer.submitTextureQuad(
		{ m_stackPos.x + e.padding[GUI_LEFT]- scale, m_stackPos.y + e.padding[GUI_RIGHT]- scale ,m_z_pos },
		{ e.width - e.widthPadding()+ scale*2,e.height - e.heightPadding()+ scale*2 },
		UVQuad::build({txtOffset.x* m_item_atlas_bit,txtOffset.y* m_item_atlas_bit },{ m_item_atlas_bit ,m_item_atlas_bit }),m_item_texture);
	incrementZ();

	if (stack->size() <= 1)
		return;
	//number of items
	static TextMesh mesh(5);
	std::string number = std::to_string(stack->size());
	TextBuilder::buildMesh(number, 100000, *m_small_font->font, mesh, TextBuilder::ALIGN_RIGHT);

	renderer.push(glm::translate(glm::mat4(1.0), {
									 m_stackPos.x +  scale + e.width - 5,
									 m_stackPos.y -  scale -10,
									 m_z_pos
		}));
	renderer.submitText(mesh, m_small_font);
	renderer.pop();
}

void GUICustomRenderer::renderBlank(BatchRenderer2D& renderer, GUIBlank& e)
{
	renderRectangle(renderer, m_stackPos.x, m_stackPos.y, e.width, e.height);
}


void GUICustomRenderer::renderImage(BatchRenderer2D& renderer, GUIImage& e)
{
	if (e.image)
	{
		if (e.renderAngle != 0 || e.scale != 1)
		{
			auto mat = glm::translate(glm::mat4(1.f), { m_stackPos.x + e.width / 2, m_stackPos.y + e.height / 2, 0 });
			mat = glm::rotate(mat, e.renderAngle, glm::vec3(0, 0, 1));
			mat = glm::scale(mat, { e.scale, e.scale, 1 });
			mat = glm::translate(mat, { -m_stackPos.x - e.width / 2, -m_stackPos.y - e.height / 2, 0 });
			renderer.push(mat);
		}
		renderer.submitTextureQuad({ m_stackPos.x, m_stackPos.y, m_z_pos }, { e.width, e.height }, e.image->getUV(),
			e.image->getTexture());
		if (e.renderAngle != 0 || e.scale != 1)
			renderer.pop();
	}
}

void GUICustomRenderer::renderButton(BatchRenderer2D& renderer, GUIButton& e)
{
	renderRectangle(renderer, m_stackPos.x, m_stackPos.y, e.width, e.height);
}

void GUICustomRenderer::renderTextBox(BatchRenderer2D& renderer, GUITextBox& e)
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

		int textWidth = e.font->font->getTextWidth(e.getValue().substr(0, e.cursorPos));
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
		TextBuilder::buildMesh({ e.getValue() }, *e.font->font, e.textMesh, TextBuilder::ALIGN_LEFT,
			{ -e.textClipOffset, -1000, -e.textClipOffset + trueWidth, 2000 }, &prop);
	}

	if (wasdirty)
	{
		e.prop = prop;
		//change cursor pos
		e.cursorMesh.setChar(0,
			prop.positions.x, prop.positions.y, prop.positions.z, prop.positions.w,0xffffff00,0x00000000,
			e.font->font->getChar(e.cursorChar));
		e.cursorMesh.currentCharCount = 1;
	}

	renderRectangle(renderer,m_stackPos.x, m_stackPos.y, e.width, e.height );
	incrementZ();
	renderer.push(glm::translate(glm::mat4(1.0), {
									 m_stackPos.x + e.padding[GUI_LEFT] + e.textClipOffset,
									 m_stackPos.y + (e.height - e.font->font->lineHeight) / 2,
									 m_z_pos
		}));
	renderer.submitText(e.textMesh, e.font);


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
										 m_stackPos.y + (e.height - e.font->font->lineHeight) / 2,
										 m_z_pos
			}));
		renderer.submitText(e.cursorMesh, e.font);
		renderer.pop();
	}
}
