#pragma once
#include  "world/World.h"
#include "graphics/VertexArray.h"
#include "graphics/Program.h"
#include "glm/vec2.hpp"
#include "graphics/Texture.h"

constexpr unsigned int CHUNK_MESH_WIDTH = WORLD_CHUNK_SIZE;
constexpr unsigned int BLOCK_ATLAS_ICON_NUMBER_BIT = 1;//2 to the n icons in atlas in row
class Renderer;

class ChunkMesh
{
	struct PosVertexData
	{
		float x, y;
	};
	struct OffsetVertexData
	{
		unsigned int offset;
	};

private:
	static VertexBufferLayout s_pos_layout;
	static VertexBufferLayout s_offset_buffer_layout;
	static VertexBuffer* s_buffer;
	static Program* s_program;
	static Texture* s_texture;

public:
	static void init();
	static inline const VertexBufferLayout& getPosLayout() { return s_pos_layout; }
	static inline const VertexBufferLayout& getOffsetLayout() { return s_offset_buffer_layout; }
	static inline Program* getProgram() { return s_program; }
	static inline VertexBuffer* getVBO() { return s_buffer; }
	static inline Texture* getAtlas() { return s_texture; }



};

class ChunkMeshInstance
{
private:
	VertexArray* m_vao;
	VertexBuffer* m_vbo;
	char* m_buff;
	glm::vec2 m_pos;
	float m_scale;


public:
	ChunkMeshInstance();

	void createVBOFromChunk(const World& world, const Chunk& chunk);

	void render();

	~ChunkMeshInstance();
};


