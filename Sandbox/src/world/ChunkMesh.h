#pragma once
#include "ndpch.h"
#include "world/World.h"
#include "graphics/API/Buffer.h"
#include "graphics/API/Shader.h"
#include "graphics/API/VertexArray.h"


constexpr unsigned int BLOCK_TEXTURE_ATLAS_SIZE =32;//icons in row
constexpr unsigned int BLOCK_CORNER_ATLAS_SIZE = 8;//icons in row

constexpr unsigned int WALL_TEXTURE_ATLAS_SIZE = BLOCK_TEXTURE_ATLAS_SIZE *2;//icons in row
constexpr unsigned int WALL_CORNER_ATLAS_SIZE = BLOCK_CORNER_ATLAS_SIZE *2;//icons in row

constexpr unsigned int BLOCK_ATLAS_PIXEL_WIDTH = BLOCK_TEXTURE_ATLAS_SIZE * 8;//pixel width of texture (block is considered 8 pixels big)

//1 over factor = ratio of bordercolor section to the whole block texture atlas
constexpr unsigned int EDGE_COLOR_TRANSFORMATION_FACTOR = 4;//how much should i divide texture pos to get to the border color



class Renderer;

class ChunkMesh
{

private:
	static VertexBufferLayout s_layout;
	
	static ShaderPtr s_program;
	
	static Texture* s_texture;
	static Texture* s_texture_corners;

public:

	struct PosVertexData
	{
		glm::vec2 pos;
		glm::vec2 uv_0;
		glm::vec2 uv_1;
	};
	
	static void init();
	static inline const VertexBufferLayout& getLayout() { return s_layout; }
	static inline ShaderPtr getProgram() { return s_program; }
	static inline Texture* getAtlas() { return s_texture; }
	static inline Texture* getCornerAtlas() { return s_texture_corners; }
};
class ChunkMeshInstance;

constexpr int BUFF_LIGHT_SIZE = WORLD_CHUNK_AREA;
constexpr int BUFF_BLOCK_SIZE = WORLD_CHUNK_AREA * 6 * (sizeof(ChunkMesh::PosVertexData));
constexpr int BUFF_WALL_SIZE = WORLD_CHUNK_AREA * 4 * 6 * (sizeof(ChunkMesh::PosVertexData));

class ChunkMeshes;
class ChunkMeshInstance
{
private:
	ChunkMeshes& m_meshes;
	uint8_t* m_light_cache;
	uint8_t* m_block_buff;
	uint8_t* m_wall_buff;
	glm::vec2 m_pos;
	int m_mesh_index;

public:
	bool enabled;
	ChunkMeshInstance(ChunkMeshes& meshes,int meshIndex);

	void updatePointers();
	
	void updateMesh(const World& world, const Chunk& chunk);

	inline glm::vec2& getPos() { return m_pos; }
	inline int getIndex()const { return m_mesh_index; }

	inline uint8_t* getLightCache() { return m_light_cache; }
};


/***
 * Manages all chunkmeshes using only one big buffer and vbo that contains blocks walls and light cache
 */
class ChunkMeshes
{
private:
	std::vector<ChunkMeshInstance*> m_instances;
	int m_chunk_count;

	uint8_t* m_light_cache;

	VertexArray* m_vao;
	VertexBuffer* m_vbo;

	uint8_t* m_block_buff;
	uint8_t* m_wall_buff;


public:
	ChunkMeshes(int defaultChunkCount);
	~ChunkMeshes();
	void reserve(int chunkCount);
	void resize(int chunkCount);
	ChunkMeshInstance* getFreeChunk();

	inline VertexArray& getVAO() { return *m_vao; }

	inline uint8_t* getLightBuffer(int index) { return m_light_cache + (index * BUFF_LIGHT_SIZE); }
	inline uint8_t* getBlockBuffer(int index) { return m_block_buff + (index * BUFF_BLOCK_SIZE); }
	inline uint8_t* getWallBuffer(int index) { return m_wall_buff + (index * BUFF_WALL_SIZE); }

	inline int getVertexOffsetToBlockBuffer(int index) { return WORLD_CHUNK_AREA * 6 * index; }
	inline int getVertexOffsetToWallBuffer(int index) { return 6*WORLD_CHUNK_AREA*m_chunk_count+ 4 * WORLD_CHUNK_AREA * 6 * index; }

	//uploads data from buffer to vbo
	void updateVBO(int index);
	auto& getChunks() { return m_instances; }

};
