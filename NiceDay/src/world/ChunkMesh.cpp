#include "ndpch.h"
#include "world/block/BlockRegistry.h"
#include "world/World.h"
#include "graphics/Renderer.h"
#include "ChunkMesh.h"
#include "glm/gtx/io.hpp"
#include "block/Block.h"
#include "WorldRenderManager.h"


VertexBufferLayout ChunkMesh::s_pos_layout;
VertexBufferLayout ChunkMesh::s_offset_buffer_layout;
VertexBuffer* ChunkMesh::s_position_vbo;
VertexBuffer* ChunkMesh::s_wall_position_vbo;
Shader* ChunkMesh::s_program;
Texture* ChunkMesh::s_texture;
Texture* ChunkMesh::s_texture_corners;



void ChunkMesh::init()
{
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;


		TextureInfo info;
		s_texture = new Texture(info.path("res/images/atlas/newatlas.png").filterMode(GL_NEAREST));
		s_texture_corners = new Texture(info.path("res/images/atlas/corners.png"));

		s_program = new Shader("res/shaders/Chunk.shader");
		s_program->bind();
		s_program->setUniform1i("u_texture", 0);
		s_program->setUniform1i("u_corners", 1);
		s_program->setUniform1i("u_texture_atlas_pixel_width_corner", BORDER_COLOR_TRANSFORMATION_FACTOR);//scale factor of determining color of corner border (4 means divide pixel pos by 4 to get to the border color)

		//todo when changing blockpixels size this wont work you need to specify pixel size of texture
		s_program->setUniform1i("u_texture_atlas_pixel_width", BLOCK_ATLAS_PIXEL_WIDTH);//for every block we have 8 pixels in texture
		s_program->unbind();

		s_pos_layout.push<float>(2);//pos
		s_offset_buffer_layout.push<unsigned int>(1);//offset
		s_offset_buffer_layout.push<unsigned int>(1);//offset corner

		//pos
		char* ray = new char[WORLD_CHUNK_AREA * 4 * 2 * sizeof(float)];
		for (int y = 0; y < WORLD_CHUNK_SIZE; y++) {
			for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
			{
				*(float*)(&ray[(y * WORLD_CHUNK_SIZE + x) * 2 * sizeof(float)]) = (float)x;
				*(float*)(&ray[(y * WORLD_CHUNK_SIZE + x) * 2 * sizeof(float) + sizeof(float)]) = (float)y;
			}
		}
		s_position_vbo = VertexBuffer::create(ray, WORLD_CHUNK_AREA * 2 * sizeof(float), GL_STATIC_DRAW);

		//wall pos
		for (int y = 0; y < WORLD_CHUNK_SIZE * 2; y++) {
			for (int x = 0; x < WORLD_CHUNK_SIZE * 2; x++)
			{
				*(float*)(&ray[(y * WORLD_CHUNK_SIZE * 2 + x) * 2 * sizeof(float)]) = (float)x;
				*(float*)(&ray[(y * WORLD_CHUNK_SIZE * 2 + x) * 2 * sizeof(float) + sizeof(float)]) = (float)y;
			}
		}
		s_wall_position_vbo = VertexBuffer::create(ray, WORLD_CHUNK_AREA * 4 * 2 * sizeof(float), GL_STATIC_DRAW);

		delete[] ray;

	}
}


ChunkMeshInstance::ChunkMeshInstance() :m_enabled(false)
{
	m_light_cache = new uint8_t[WORLD_CHUNK_AREA];
	const int BUFF_SIZE = WORLD_CHUNK_AREA * sizeof(int) * 2;
	const int WALL_BUFF_SIZE = WORLD_CHUNK_AREA * sizeof(int) * 2 * 4;
	m_buff = new char[BUFF_SIZE];
	memset(m_buff, 0, BUFF_SIZE);
	m_wall_buff = new char[WALL_BUFF_SIZE];
	memset(m_wall_buff, 0, WALL_BUFF_SIZE);

	m_vao = VertexArray::create();
	m_vbo = VertexBuffer::create(m_buff, BUFF_SIZE, GL_DYNAMIC_DRAW);

	m_vao->addBuffer(*ChunkMesh::getVBO(), ChunkMesh::getPosLayout());
	m_vao->addBuffer(*m_vbo, ChunkMesh::getOffsetLayout());

	m_wall_vao = VertexArray::create();
	m_wall_vbo = VertexBuffer::create(m_wall_buff, WALL_BUFF_SIZE, GL_DYNAMIC_DRAW);

	m_wall_vao->addBuffer(*ChunkMesh::getWallVBO(), ChunkMesh::getPosLayout());
	m_wall_vao->addBuffer(*m_wall_vbo, ChunkMesh::getOffsetLayout());
}


void ChunkMeshInstance::updateMesh(const World& world, const Chunk& chunk)
{
	for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
	{
		int ylevel = y * WORLD_CHUNK_SIZE;
		for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
		{
			const BlockStruct& bs = chunk.getBlock(x, y);
			const Block& blok = BlockRegistry::get().getBlock(bs.block_id);
			auto t_offset = 1 + blok.getTextureOffset(x, y, bs);
			auto t_corner_offset = blok.getCornerOffset(x, y, bs);

			*((unsigned int*)&m_buff[sizeof(int) * 2 * (ylevel + x)]) = t_offset;
			*((unsigned int*)&m_buff[sizeof(int) * 2 * (ylevel + x) + sizeof(int)]) = t_corner_offset;
		}
	}
	for (int y = 0; y < WORLD_CHUNK_SIZE * 2; y++)
	{
		for (int x = 0; x < WORLD_CHUNK_SIZE * 2; x++)
		{
			const BlockStruct& bs = chunk.getBlock(x / 2, y / 2);

			const Wall& wall = BlockRegistry::get().getWall(bs.wall_id[(y & 1) * 2 + (x & 1)]);
			auto t_offset = 1+wall.getTextureOffset(x, y,bs);
			auto t_corner_offset = wall.getCornerOffset(x,y, bs);

			*((unsigned int*)&m_wall_buff[sizeof(int) * 2 * (y*WORLD_CHUNK_SIZE*2 + x)]) = t_offset;
			*((unsigned int*)&m_wall_buff[sizeof(int) * 2 * (y*WORLD_CHUNK_SIZE*2 + x)+sizeof(int)]) = t_corner_offset;
		}

	}

	m_vbo->changeData(m_buff, WORLD_CHUNK_AREA * sizeof(int) * 2, 0);
	m_wall_vbo->changeData(m_wall_buff, WORLD_CHUNK_AREA * sizeof(int) * 2 * 4, 0);
}


ChunkMeshInstance::~ChunkMeshInstance()
{
	delete[] m_light_cache;
	delete m_vao;
	delete[] m_buff;
	delete m_vbo;
}
