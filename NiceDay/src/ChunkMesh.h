#pragma once
#include "graphics/IRenderable.h"
#include  "world/World.h"
class ChunkMesh: public IRenderable
{
private:
	VertexArray* m_vao;
	VertexBuffer* m_vbo;
	char* m_buff;
	int m_width;
	VertexBufferLayout m_layout;
	Program* m_program;

public:
	ChunkMesh(unsigned int width);

	void createVBOFromChunk(const World& world,const Chunk& chunk);

	virtual void render(Renderer& r) override;

	~ChunkMesh();
};

