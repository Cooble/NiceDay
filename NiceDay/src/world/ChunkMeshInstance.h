#pragma once
#include  "world/World.h"
#include "graphics/Program.h"
#include "glm/vec2.hpp"
#include "graphics/Texture.h"
#include "graphics/buffer/VertexBufferLayout.h"
#include "graphics/buffer/VertexBuffer.h"
#include "graphics/buffer/VertexArray.h"

constexpr unsigned int BLOCK_TEXTURE_ATLAS_SIZE_BIT = 1;//2 to the n icons in atlas in row
constexpr unsigned int BLOCK_CORNER_ATLAS_SIZE_BIT = 3;//2 to the n icons in atlas in row
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
	static Texture* s_texture_corners;

public:
	static void init();
	static inline const VertexBufferLayout& getPosLayout() { return s_pos_layout; }
	static inline const VertexBufferLayout& getOffsetLayout() { return s_offset_buffer_layout; }
	static inline Program* getProgram() { return s_program; }
	static inline VertexBuffer* getVBO() { return s_buffer; }
	static inline Texture* getAtlas() { return s_texture; }
	static inline Texture* getCornerAtlas() { return s_texture_corners; }



};

class ChunkMeshInstance
{
private:
	VertexArray* m_vao;
	VertexBuffer* m_vbo;
	char* m_buff;
	glm::vec2 m_pos;

public:
	bool m_enabled;
	ChunkMeshInstance();

	void updateMesh(const World& world, const Chunk& chunk);

	inline glm::vec2& getPos() { return m_pos; }
	inline VertexArray& getVAO() { return *m_vao; }

	~ChunkMeshInstance();

};


