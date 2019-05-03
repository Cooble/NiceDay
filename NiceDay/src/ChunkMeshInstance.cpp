#include "ndpch.h"
#include "world/BlockRegistry.h"
#include "world/World.h"
#include "graphics/Renderer.h"
#include "ChunkMeshInstance.h"
#include "glm/gtx/io.hpp"


VertexBufferLayout ChunkMesh::s_pos_layout;
VertexBufferLayout ChunkMesh::s_offset_buffer_layout;
VertexBuffer* ChunkMesh::s_buffer;
Program* ChunkMesh::s_program;
Texture* ChunkMesh::s_texture;

void ChunkMesh::init()
{
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;
		
		static char* ray = new char[CHUNK_MESH_WIDTH*CHUNK_MESH_WIDTH * 2 * sizeof(float)];

		s_texture = new Texture("res/images/atlas.png", GL_NEAREST);
		s_program = new Program("res/shaders/Chunk.shader");
		s_program->bind();
		s_program->setUniform1i("u_texture", 0);
		s_program->setUniform1i("u_atlas_icon_number_bit", BLOCK_ATLAS_ICON_NUMBER_BIT);
		s_program->unbind();

		s_pos_layout.push<float>(2);//pos
		s_offset_buffer_layout.push<unsigned int>(1);

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
static int flipBits(int f)
{
	int out = 0;
	for(int i = 0;i<32;i++)
	{
		out |= (f  >> i) & 1;
		out <<= 1;
	}
	return  out;
}
static char flipBits(char f)
{
	char out = 0;
	for (int i = 0; i < 8; i++)
	{
		out |= (f >> i) & 1;
		out <<= 1;
	}
	return out;
}

ChunkMeshInstance::ChunkMeshInstance()
{
	m_buff = new char[CHUNK_MESH_WIDTH*CHUNK_MESH_WIDTH * sizeof(int)];
	memset(m_buff,flipBits((char)3), CHUNK_MESH_WIDTH*CHUNK_MESH_WIDTH * sizeof(int));
	/*for(int i =0 ;i< CHUNK_MESH_WIDTH*CHUNK_MESH_WIDTH;i++)
	{
		*(unsigned int *)&m_buff[i * sizeof(int)] = 1;
	}*/

	m_vao = new VertexArray();
	m_vbo = new VertexBuffer(m_buff, CHUNK_MESH_WIDTH * CHUNK_MESH_WIDTH * sizeof(int), false);

	m_vao->addBuffer(*ChunkMesh::getVBO(), ChunkMesh::getPosLayout());
	VertexBufferLayout b;
	b.push<unsigned int>(1);
	m_vao->addBuffer(*m_vbo, b);
}


void ChunkMeshInstance::createVBOFromChunk(const World& world, const Chunk& chunk)
{
	m_pos = glm::vec2(-1, -1);
	m_scale = 0.04f;

	for (int y = 0; y < CHUNK_MESH_WIDTH; y++)
	{
		for (int x = 0; x < CHUNK_MESH_WIDTH; x++)
		{
			auto bs = chunk.getBlock(x, y);
			auto t_offset = BlockRegistry::get().getBlock(bs.id).getTextureOffset(bs.metadata);
			//auto t_offset = 0;

			*((unsigned int*)&m_buff[sizeof(unsigned int)*(y*CHUNK_MESH_WIDTH + x)]) = t_offset;
			//*((unsigned int*)&m_buff[0]) = t_offset;
		}
	}
	//m_vbo->changeData(m_buff, CHUNK_MESH_WIDTH*CHUNK_MESH_WIDTH*sizeof(unsigned int), 0);
}

void ChunkMeshInstance::render()
{
	auto& program = *ChunkMesh::getProgram();
	program.bind();
	ChunkMesh::getAtlas()->bind(0);
	glm::mat4 trans(1.0f);
	m_scale = 2;
	trans = glm::translate(trans, glm::vec3(m_pos.x, m_pos.y, 0.0f));
	trans = glm::scale(trans, glm::vec3(m_scale, m_scale, 1));
	program.setUniformMat4("u_transform", trans);

	m_vao->bind();
	Call(glDrawArrays(GL_POINTS, 0, 1/*CHUNK_MESH_WIDTH * CHUNK_MESH_WIDTH*/));
}


ChunkMeshInstance::~ChunkMeshInstance()
{
	delete m_vao;
	delete[] m_buff;
	delete m_vbo;
}
