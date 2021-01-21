#pragma once
#include "gui/GUIRenderer.h"
#include "graphics/TextureAtlas.h"

class GUIItemContainer;
constexpr glm::vec4 guiCRColor = { 0.286, 0.369, 0.671 ,1 };

class GUICustomRenderer :public GUIRenderer
{
private:
	//====window sprites
	Sprite m_corn_left_down;
	Sprite m_corn_right_down;
	Sprite m_corn_left_up;
	Sprite m_corn_right_up;
	Sprite m_left;
	Sprite m_up;
	Sprite m_right;
	Sprite m_down;
	Sprite m_center;
	//====trash
	Sprite m_trash;
	//====window sprites
	int m_item_atlas_size;
	float m_item_atlas_bit;
	Texture* m_item_texture;
	FontMaterial* m_small_font;
	
public:
	GUICustomRenderer(glm::vec2 windowSize = { 1280,720 });

	void setItemAtlas(int atlasSize,Texture* t);
	void render(BatchRenderer2D& renderer) override;
	void renderRectangle(BatchRenderer2D& renderer, float x, float y, float width, float height);
	void renderElement(BatchRenderer2D& renderer, GUIElement& e) override;
	void renderWindow(BatchRenderer2D& renderer, GUIWindow& e) override;
	void renderItemContainer(BatchRenderer2D& renderer, GUIItemContainer& e);
	void renderBlank(BatchRenderer2D& renderer, GUIBlank& e) override;
	void renderImage(BatchRenderer2D& renderer, GUIImage& e) override;
	void renderButton(BatchRenderer2D& renderer, GUIButton& e) override;
	void renderTextBox(BatchRenderer2D& renderer, GUITextBox& e) override;
};


