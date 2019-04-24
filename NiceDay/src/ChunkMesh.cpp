#include "ndpch.h"
#include "ChunkMesh.h"
#include "world/BlockRegistry.h"
#include "world/World.h"

#define VERTEX_BYTE_SIZE 2*sizeof(float)+sizeof(int)
//todo from chunkmesh make chunkmesh instance you dont need to have 100000 same programs
ChunkMesh::ChunkMesh(unsigned int width) :m_width(width)
{
	m_program = new Program("res/shaders/Chunk.shader");
	m_vao = new VertexArray();
	m_layout.push<float>(2);//pos
	m_layout.push<unsigned int>(1);//texture_offset
	m_vbo = new VertexBuffer(nullptr, width*width*(VERTEX_BYTE_SIZE), true);
	m_vao->addBuffer(*m_vbo, m_layout);
	m_buff = new char[width*width];

	for (int y = 0; y < m_width; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			*((float*)&m_buff[VERTEX_BYTE_SIZE*(y*m_width + x)]) = (float)x / m_width;//set first 4 bytes to float x
			*((float*)&m_buff[VERTEX_BYTE_SIZE*(y*m_width + x) + sizeof(float)]) = (float)y / m_width;//set next 4 bytes to float y
		}
	}


}
void ChunkMesh::createVBOFromChunk(const World& world, const Chunk& chunk)
{

	//int offset = chunk.offset;

	for (int y = 0; y < m_width; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			auto bs = chunk.getBlock(x, y);
			auto t_offset = BlockRegistry::get().getBlock(bs.id).getTextureOffset(bs.metadata);
			*((unsigned int*)&m_buff[VERTEX_BYTE_SIZE*(y*m_width + x) + 2 * sizeof(float)]) = t_offset;
		}
	}
}

void ChunkMesh::render(Renderer & r)
{
	m_program->bind();
	m_vao->bind();
	Call(glDrawArrays(GL_POINTS, 0, m_width * m_width));

}


ChunkMesh::~ChunkMesh()
{
	delete m_vao;
	delete m_buff;
	delete m_vbo;
	delete m_program;
}
