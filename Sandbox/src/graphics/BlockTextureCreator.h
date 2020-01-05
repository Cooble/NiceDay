#pragma once

#include "graphics/Effect.h"

class Block;

class BlockTextureCreator
{
private:
	FrameBufferTexturePair m_fbo;
	VertexBuffer* m_vbo;
	VertexArray* m_vao;
public:
	void createTextures();

	void createTexture(const Block& block);
};
