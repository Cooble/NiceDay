#pragma once
#include "Renderable2D.h"
#include "IBatchRenderable2D.h"

namespace nd {

using namespace glm;

class SpriteSheetResource
{
private:
	Texture* m_texture; //todo maybe create texture right here to avoid cache misses..
	const uint32_t m_width_icons;
	const uint32_t m_height_icons;
public:
	//width - means how many subimages texture contains in one row
	//height - means how many subimages texture contains in one column
	SpriteSheetResource(Texture* t, uint32_t width, uint32_t height);
	inline Texture* getTexture() const { return m_texture; }
	inline uint32_t getIconsWidth() const { return m_width_icons; }
	inline uint32_t getIconsHeight() const { return m_height_icons; }
};

class Sprite : public Renderable2D, IBatchRenderable2D
{
private:
	SpriteSheetResource* m_resource;
	UVQuad m_uv_quad;
	vec3 m_position;
	vec2 m_size;
	bool m_enabled;
public:
	Sprite(SpriteSheetResource* r);
	Sprite();


	void setSpriteIndex(uint32_t u, uint32_t v, bool horizontalFlip = false, bool verticalFlip = false,
	                    bool rotate90 = false);

	inline void setPosition(const glm::vec3& pos) { m_position = pos; }
	inline void setSize(const glm::vec2& size) { m_size = size; }
	inline const Texture* getTexture() const override { return m_resource->getTexture(); }
	inline const vec3& getPosition() const override { return m_position; }
	inline const vec2& getSize() const override { return m_size; }
	inline bool isEnabled() const { return m_enabled; }
	inline void setEnabled(bool enabled) { m_enabled = enabled; }
	inline const UVQuad& getUV() const override { return m_uv_quad; }
	inline UVQuad& getUV() { return m_uv_quad; }
	void render(BatchRenderer2D&) override;
};

class Animation : public Sprite
{
private:
	std::vector<int> m_indexes;
	int m_current_index = 0;
	bool m_repeat_saw = false;
	int m_increase_index = 1;

	bool m_horizontalFlip;
	bool m_verticalFlip;


public:
	Animation(SpriteSheetResource* r, std::initializer_list<int> uvs = {}, bool repeatSaw = true,
	          bool horizontalFlip = false, bool verticalFlip = false);
	Animation() = default;
	// resets animation to index 0
	// need call nextFrame to refresh image
	void reset();

	// need call updateAfterFlip() or nextFrame() to refreshImage
	inline void setHorizontalFlip(bool v) { m_horizontalFlip = v; }

	// need call updateAfterFlip() or nextFrame() to refreshImage
	inline void setVerticalFlip(bool v) { m_verticalFlip = v; }

	// after changing flipping need to call this to refresh image
	void updateAfterFlip();

	// same as setSpriteIndex()
	void setSpriteFrame(int u, int v);

	void nextFrame();
};
}
