#include "ndpch.h"
#include "world/block/BlockRegistry.h"
#include "world/World.h"
#include "ChunkMesh.h"
#include "glm/gtx/io.hpp"
#include "block/Block.h"
#include "WorldRenderManager.h"


VertexBufferLayout ChunkMesh::s_layout;

ShaderPtr ChunkMesh::s_program;
Texture* ChunkMesh::s_texture;
Texture* ChunkMesh::s_texture_corners;



void ChunkMesh::init()
{
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;


		TextureInfo info;
		s_texture = Texture::create(info.path("res/images/blockAtlas/atlas.png").filterMode(TextureFilterMode::NEAREST));
		s_texture_corners = Texture::create(info.path("res/images/atlas/corners.png"));

		s_program = ShaderLib::loadOrGetShader("res/shaders/ChunkNew.shader");
		s_program->bind();
		std::static_pointer_cast<GLShader>(s_program)->setUniform1i("u_texture", 0);
		std::static_pointer_cast<GLShader>(s_program)->setUniform1i("u_corners", 1);
		std::static_pointer_cast<GLShader>(s_program)->setUniform1i("u_texture_atlas_pixel_width_corner", EDGE_COLOR_TRANSFORMATION_FACTOR);//scale factor of determining color of corner border (4 means divide pixel pos by 4 to get to the border color)

		//todo when changing blockpixels size this wont work you need to specify pixel size of texture
		std::static_pointer_cast<GLShader>(s_program)->setUniform1i("u_texture_atlas_pixel_width", BLOCK_ATLAS_PIXEL_WIDTH);//for every block we have 8 pixels in texture
		s_program->unbind();

		s_layout.push<float>(2);//pos
		s_layout.push<float>(2);//uv0
		s_layout.push<float>(2);//uv1

		/*
		//pos
		char* ray = new char[WORLD_CHUNK_AREA * 4 * 2 * sizeof(float)];
		for (int y = 0; y < WORLD_CHUNK_SIZE; y++) {
			for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
			{
				*(float*)(&ray[(y * WORLD_CHUNK_SIZE + x) * 2 * sizeof(float)]) = (float)x;
				*(float*)(&ray[(y * WORLD_CHUNK_SIZE + x) * 2 * sizeof(float) + sizeof(float)]) = (float)y;
			}
		}
		m_vbo = VertexBuffer::create(ray, WORLD_CHUNK_AREA * 2 * sizeof(float));

		//wall pos
		for (int y = 0; y < WORLD_CHUNK_SIZE * 2; y++) {
			for (int x = 0; x < WORLD_CHUNK_SIZE * 2; x++)
			{
				*(float*)(&ray[(y * WORLD_CHUNK_SIZE * 2 + x) * 2 * sizeof(float)]) = (float)x;
				*(float*)(&ray[(y * WORLD_CHUNK_SIZE * 2 + x) * 2 * sizeof(float) + sizeof(float)]) = (float)y;
			}
		}
		s_wall_vbo = VertexBuffer::create(ray, WORLD_CHUNK_AREA * 4 * 2 * sizeof(float));

		delete[] ray;
		*/
	}
}

ChunkMeshes::ChunkMeshes(int defaultChunkCount):m_chunk_count(0),m_light_cache(nullptr)
{
	reserve(defaultChunkCount);
}

ChunkMeshes::~ChunkMeshes()
{
	if (m_light_cache) {
		delete[] m_light_cache;
		delete m_vbo;
		delete m_vao;
		for (auto instance : m_instances)
			delete instance;
	}
}

void ChunkMeshes::reserve(int chunkCount)
{
	if(m_chunk_count<chunkCount)
	{
		m_chunk_count = chunkCount;
		if (m_light_cache) {
			delete[] m_light_cache;
			delete m_vbo;
			delete m_vao;

		}
		//everything is contained in one big countinous array
		auto BIG_BUFFER_SIZE = chunkCount * BUFF_LIGHT_SIZE + chunkCount * BUFF_BLOCK_SIZE + chunkCount * BUFF_WALL_SIZE;
		m_light_cache = new uint8_t[BIG_BUFFER_SIZE];
		m_block_buff = m_light_cache + chunkCount * BUFF_LIGHT_SIZE;
		m_wall_buff = m_block_buff + chunkCount * BUFF_BLOCK_SIZE;
		ZeroMemory(m_light_cache, BIG_BUFFER_SIZE);

		while (m_instances.size() < chunkCount)
			m_instances.push_back(new ChunkMeshInstance(*this, m_instances.size()));
		for (auto instance : m_instances)
			instance->updatePointers();

		m_vbo = VertexBuffer::create(m_block_buff, chunkCount * BUFF_BLOCK_SIZE + chunkCount * BUFF_WALL_SIZE, BufferUsage::DYNAMIC_DRAW);
		m_vbo->setLayout(ChunkMesh::getLayout());
		m_vao = VertexArray::create();
		m_vao->addBuffer(*m_vbo);
	}
}

void ChunkMeshes::resize(int chunkCount)
{
	//todo;
	reserve(chunkCount);
}

ChunkMeshInstance* ChunkMeshes::getFreeChunk()
{
	for (auto instance : m_instances)
		if(!instance->enabled)
		{
			instance->enabled = true;
			return instance;
		}
	reserve(m_instances.size() + 2);
	return getFreeChunk();
}

void ChunkMeshes::updateVBO(int index)
{
	m_vbo->changeData((char*)(m_block_buff+(index * BUFF_BLOCK_SIZE)), BUFF_BLOCK_SIZE, BUFF_BLOCK_SIZE * index);
	m_vbo->changeData((char*)(m_wall_buff+(index * BUFF_WALL_SIZE)), BUFF_WALL_SIZE, BUFF_BLOCK_SIZE * m_chunk_count+index*BUFF_WALL_SIZE);
}

ChunkMeshInstance::ChunkMeshInstance(ChunkMeshes& meshes, int meshIndex)
:m_meshes(meshes),m_mesh_index(meshIndex),enabled(false){
	updatePointers();
}
//god knows why map is slower
#define CHUNK_MESSH_USEMAP 0 
#if CHUNK_MESSH_USEMAP==1
void ChunkMeshInstance::updateMesh(const World& world, const Chunk& chunk)
{
	//TimerStaper t("updateMesh MAP");
	m_vbo->bind();
	auto* buff = (unsigned int*)m_vbo->mapPointer();
	for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
	{
		int ylevel = y * WORLD_CHUNK_SIZE;
		for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
		{
			const BlockStruct& bs = chunk.getBlock(x, y);
			const Block& blok = BlockRegistry::get().getBlock(bs.block_id);
			auto t_offset = 1 + blok.getTextureOffset(x, y, bs);
			auto t_corner_offset = blok.getCornerOffset(x, y, bs);

			//*((unsigned int*)&blockBuff[sizeof(int) * 2 * (ylevel + x)]) = t_offset;
			//*((unsigned int*)&blockBuff[sizeof(int) * 2 * (ylevel + x) + sizeof(int)]) = t_corner_offset;
			buff[2 * (ylevel + x)] = t_offset;
			buff[2 * (ylevel + x) + 1] = t_corner_offset;
		}
	}
	m_vbo->unMapPointer();

	m_wall_vbo->bind();
	buff = (unsigned int*)m_wall_vbo->mapPointer();
	for (int y = 0; y < WORLD_CHUNK_SIZE * 2; y++)
	{
		for (int x = 0; x < WORLD_CHUNK_SIZE * 2; x++)
		{
			const BlockStruct& bs = chunk.getBlock(x / 2, y / 2);

			const Wall& wall = BlockRegistry::get().getWall(bs.wall_id[(y & 1) * 2 + (x & 1)]);
			auto t_offset = 1+wall.getTextureOffset(x, y,bs);
			auto t_corner_offset = wall.getCornerOffset(x,y, bs);

			//*((unsigned int*)&wallBuff[sizeof(int) * 2 * (y*WORLD_CHUNK_SIZE*2 + x)]) = t_offset;
			//*((unsigned int*)&wallBuff[sizeof(int) * 2 * (y*WORLD_CHUNK_SIZE*2 + x)+sizeof(int)]) = t_corner_offset;

			buff[2 * (y*WORLD_CHUNK_SIZE * 2 + x)] = t_offset;
			buff[2 * (y*WORLD_CHUNK_SIZE * 2 + x) + 1] = t_corner_offset;
		}

	}

	m_wall_vbo->unMapPointer();
	m_wall_vbo->unbind();
}
#else

void ChunkMeshInstance::updateMesh(const World& world, const Chunk& chunk)
{
	//SCOPE_MEASURE("mesh update");

	float co = 1.0f / BLOCK_TEXTURE_ATLAS_SIZE;
	float co_corner = 1.0f / BLOCK_CORNER_ATLAS_SIZE;
	
	for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
	{
		int ylevel = y * WORLD_CHUNK_SIZE;
		for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
		{
			const BlockStruct& bs = chunk.block(x, y);
			const Block& blok = BlockRegistry::get().getBlock(bs.block_id);
			half_int t_offset = blok.getTextureOffset(x, y, bs);
			half_int t_corner_offset = blok.getCornerOffset(x, y, bs);
			auto point = (ChunkMesh::PosVertexData*)&m_block_buff[(sizeof(ChunkMesh::PosVertexData)*(y*WORLD_CHUNK_SIZE+x))*6];
			int oldX = x;
			if(t_offset==-1)
				x = -1000;//discard quad
			
			//0,0
			{
				point->pos = glm::vec2(x, y);
				//check if t_offset == -1 -> discard
				point->uv_0 = glm::vec2(t_offset.x,			t_offset.y) * co;
				point->uv_1 = glm::vec2(t_corner_offset.x,	t_corner_offset.y) * co_corner;
			}
			++point;
			//1,0
			{
				point->pos = glm::vec2(x+1, y);
				//check if t_offset == -1 -> discard
				point->uv_0 = glm::vec2(t_offset.x+1, t_offset.y) * co;
				point->uv_1 = glm::vec2(t_corner_offset.x+1, t_corner_offset.y) * co_corner;
			}
			++point;
			//1,1
			{
				point->pos = glm::vec2(x + 1, y+1);
				//check if t_offset == -1 -> discard
				point->uv_0 = glm::vec2(t_offset.x + 1, t_offset.y+1) * co;
				point->uv_1 = glm::vec2(t_corner_offset.x + 1, t_corner_offset.y+1) * co_corner;
			}
			++point;
			//0,0
			{
				auto pp = point - 3;
				point->pos = pp->pos;
				point->uv_0 = pp->uv_0;
				point->uv_1 = pp->uv_1;
			}
			++point;
			//1,1
			{
				auto pp = point - 2;
				point->pos = pp->pos;
				point->uv_0 = pp->uv_0;
				point->uv_1 = pp->uv_1;
			}
			++point;
			//0,1
			{
				point->pos = glm::vec2(x, y + 1);
				//check if t_offset == -1 -> discard
				point->uv_0 = glm::vec2(t_offset.x, t_offset.y + 1) * co;
				point->uv_1 = glm::vec2(t_corner_offset.x, t_corner_offset.y + 1) * co_corner;
			}
			x = oldX;
		}
	}

	co = 1.0f / WALL_TEXTURE_ATLAS_SIZE;
	co_corner = 1.0f / WALL_CORNER_ATLAS_SIZE;
	
	for (int y = 0; y < WORLD_CHUNK_SIZE * 2; y++)
	{
		for (int x = 0; x < WORLD_CHUNK_SIZE * 2; x++)
		{
			auto& bs = chunk.block(x / 2, y / 2);

			auto& wall = BlockRegistry::get().getWall(bs.wall_id[(y & 1) * 2 + (x & 1)]);
			half_int t_offset = wall.getTextureOffset(x, y, bs);
			half_int t_corner_offset = wall.getCornerOffset(x, y, bs);

			auto point = (ChunkMesh::PosVertexData*)&m_wall_buff[sizeof(ChunkMesh::PosVertexData) * (y * WORLD_CHUNK_SIZE*2 + x) * 6];
			int oldX = x;
			if (t_offset == -1)
			{
				x = -1000;//discard quad
			}
			//0,0
			{
				point->pos = glm::vec2(x, y);
				//check if t_offset == -1 -> discard
				point->uv_0 = glm::vec2(t_offset.x, t_offset.y) * co;
				point->uv_1 = glm::vec2(t_corner_offset.x, t_corner_offset.y) * co_corner;
			}
			++point;
			//1,0
			{
				point->pos = glm::vec2(x + 1, y);
				//check if t_offset == -1 -> discard
				point->uv_0 = glm::vec2(t_offset.x + 1, t_offset.y) * co;
				point->uv_1 = glm::vec2(t_corner_offset.x + 1, t_corner_offset.y) * co_corner;
			}
			++point;
			//1,1
			{
				point->pos = glm::vec2(x + 1, y + 1);
				//check if t_offset == -1 -> discard
				point->uv_0 = glm::vec2(t_offset.x + 1, t_offset.y + 1) * co;
				point->uv_1 = glm::vec2(t_corner_offset.x + 1, t_corner_offset.y + 1) * co_corner;
			}
			++point;
			//0,0
			{
				auto pp = point - 3;
				point->pos = pp->pos;
				point->uv_0 = pp->uv_0;
				point->uv_1 = pp->uv_1;
			}
			++point;
			//1,1
			{
				auto pp = point - 2;
				point->pos = pp->pos;
				point->uv_0 = pp->uv_0;
				point->uv_1 = pp->uv_1;
			}
			++point;
			//0,1
			{
				point->pos = glm::vec2(x, y + 1);
				//check if t_offset == -1 -> discard
				point->uv_0 = glm::vec2(t_offset.x, t_offset.y + 1) * co;
				point->uv_1 = glm::vec2(t_corner_offset.x, t_corner_offset.y + 1) * co_corner;
			}
			x = oldX;
		}

		
	}

	{
		//SCOPE_MEASURE("changemeshdata to vbo");
		m_meshes.updateVBO(m_mesh_index);
	}
}
#endif



void ChunkMeshInstance::updatePointers()
{
	m_block_buff = m_meshes.getBlockBuffer(m_mesh_index);
	m_wall_buff = m_meshes.getWallBuffer(m_mesh_index);
	m_light_cache = m_meshes.getLightBuffer(m_mesh_index);
}
