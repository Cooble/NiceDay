#pragma once

#include "graphics/Effect.h"

class Wall;
class Block;

class BlockTextureCreator
{
private:
	nd::FrameBufferTexturePair m_fbo;
	nd::VertexBuffer* m_vbo;
	nd::VertexArray* m_vao;
public:
	void createTextures();

	void createTexture(const Block& block);
	void createTexture(const Wall& wall);
};
