#include "ndpch.h"
#include "world/BlockRegistry.h"
#include "world/World.h"
#include "graphics/Renderer.h"
#include "ChunkMeshInstance.h"
#include "glm/gtx/io.hpp"
#include "block/Block.h"


VertexBufferLayout ChunkMesh::s_pos_layout;
VertexBufferLayout ChunkMesh::s_offset_buffer_layout;
VertexBuffer* ChunkMesh::s_buffer;
Program* ChunkMesh::s_program;
Texture* ChunkMesh::s_texture;
Texture* ChunkMesh::s_texture_corners;



void ChunkMesh::init()
{
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;

		static char* ray = new char[CHUNK_MESH_WIDTH*CHUNK_MESH_WIDTH * 2 * sizeof(float)];

		s_texture = new Texture("res/images/atlas/atlas_small.png", GL_NEAREST);
		s_texture_corners = new Texture("res/images/atlas/corners.png", GL_NEAREST);

		s_program = new Program("res/shaders/Chunk.shader");
		s_program->bind();
		s_program->setUniform1i("u_texture", 0);
		s_program->setUniform1i("u_corners", 1);
		s_program->setUniform1i("u_texture_atlas_icon_number_bit", BLOCK_TEXTURE_ATLAS_SIZE_BIT);
		s_program->setUniform1i("u_corner_atlas_icon_number_bit", BLOCK_CORNER_ATLAS_SIZE_BIT);
		s_program->unbind();

		s_pos_layout.push<float>(2);//pos
		s_offset_buffer_layout.push<unsigned int>(1);//offset
		s_offset_buffer_layout.push<unsigned int>(1);//offset corner

		for (int y = 0; y < CHUNK_MESH_WIDTH; y++) {
			for (int x = 0; x < CHUNK_MESH_WIDTH; x++)
			{
				*(float*)(&ray[(y*CHUNK_MESH_WIDTH + x) * 2 * sizeof(float)]) = (float)x;
				*(float*)(&ray[(y*CHUNK_MESH_WIDTH + x) * 2 * sizeof(float) + sizeof(float)]) = (float)y;
			}
		}
		s_buffer = new VertexBuffer(ray, CHUNK_MESH_WIDTH*CHUNK_MESH_WIDTH * 2 * sizeof(float));
	}
}


ChunkMeshInstance::ChunkMeshInstance() :m_enabled(false)
{
	const int BUFF_SIZE = CHUNK_MESH_WIDTH * CHUNK_MESH_WIDTH * sizeof(int) * 2;
	m_buff = new char[BUFF_SIZE];
	memset(m_buff, 0, BUFF_SIZE);

	m_vao = new VertexArray();
	m_vbo = new VertexBuffer(m_buff, BUFF_SIZE, false);

	m_vao->addBuffer(*ChunkMesh::getVBO(), ChunkMesh::getPosLayout());
	m_vao->addBuffer(*m_vbo, ChunkMesh::getOffsetLayout());
}


void ChunkMeshInstance::updateMesh(const World& world, const Chunk& chunk)
{
	for (int y = 0; y < CHUNK_MESH_WIDTH; y++)
	{
		int ylevel = y * CHUNK_MESH_WIDTH;
		for (int x = 0; x < CHUNK_MESH_WIDTH; x++)
		{
			const BlockStruct& bs = chunk.getBlock(x, y);
			const Block& blok = BlockRegistry::get().getBlock(bs.id);
			auto t_offset = 1 + blok.getTextureOffset(bs.metadata);
			auto t_corner_offset = blok.getCornerOffset(bs.corner);

			*((unsigned int*)&m_buff[sizeof(unsigned int)* 2 * (ylevel + x)]) = t_offset;
			*((unsigned int*)&m_buff[sizeof(unsigned int)* 2 * (ylevel + x) + sizeof(unsigned int)]) = t_corner_offset;
		}
	}
	m_vbo->changeData(m_buff, CHUNK_MESH_WIDTH*CHUNK_MESH_WIDTH * sizeof(unsigned int)*2, 0);
}




ChunkMeshInstance::~ChunkMeshInstance()
{
	delete m_vao;
	delete[] m_buff;
	delete m_vbo;
}
