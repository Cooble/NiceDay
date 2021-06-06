#pragma once
#include "gui/GUIRenderer.h"
#include "graphics/TextureAtlas.h"

class GUIItemContainer;
constexpr glm::vec4 guiCRColor = { 0.286, 0.369, 0.671 ,1 };

class GUICustomRenderer :public nd::GUIRenderer
{
private:
	//====window sprites
	nd::Sprite m_corn_left_down;
	nd::Sprite m_corn_right_down;
	nd::Sprite m_corn_left_up;
	nd::Sprite m_corn_right_up;
	nd::Sprite m_left;
	nd::Sprite m_up;
	nd::Sprite m_right;
	nd::Sprite m_down;
	nd::Sprite m_center;
	//====trash
	nd::Sprite m_trash;
	//====window sprites
	int m_item_atlas_size;
	float m_item_atlas_bit;
	nd::Texture* m_item_texture;
	nd::FontMaterial* m_small_font;
	
public:
	
	GUICustomRenderer(glm::vec2 windowSize = { 1280,720 });

	void setItemAtlas(int atlasSize, nd::Texture* t);
	void render(nd::BatchRenderer2D& renderer) override;
	void renderRectangle(nd::BatchRenderer2D& renderer, float x, float y, float width, float height);
	void renderElement(nd::BatchRenderer2D& renderer, nd::GUIElement& e) override;
	void renderWindow(nd::BatchRenderer2D& renderer, nd::GUIWindow& e) override;
	void renderItemContainer(nd::BatchRenderer2D& renderer, GUIItemContainer& e);
	void renderBlank(nd::BatchRenderer2D& renderer, nd::GUIBlank& e) override;
	void renderImage(nd::BatchRenderer2D& renderer, nd::GUIImage& e) override;
	void renderButton(nd::BatchRenderer2D& renderer, nd::GUIButton& e) override;
	void renderTextBox(nd::BatchRenderer2D& renderer, nd::GUITextBox& e) override;

	
};


