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

class ChunkMeshNew
{

private:
	static VertexBufferLayout s_layout;
	
	static Shader* s_program;
	
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
	static inline Shader* getProgram() { return s_program; }
	static inline Texture* getAtlas() { return s_texture; }
	static inline Texture* getCornerAtlas() { return s_texture_corners; }
};

class ChunkMeshInstanceNew
{
private:
	uint8_t* m_light_cache;

	VertexArray* m_vao;
	VertexArray* m_wall_vao;
	
	VertexBuffer* m_vbo;
	VertexBuffer* m_wall_vbo;
	char* m_buff;
	char* m_wall_buff;
	glm::vec2 m_pos;

public:
	bool m_enabled;
	ChunkMeshInstanceNew();

	void updateMesh(const World& world, const Chunk& chunk);

	inline glm::vec2& getPos() { return m_pos; }
	inline VertexArray& getVAO() { return *m_vao; }
	inline VertexArray& getWallVAO() { return *m_wall_vao; }
	inline uint8_t* getLightCache() { return m_light_cache; }

	~ChunkMeshInstanceNew();

};


