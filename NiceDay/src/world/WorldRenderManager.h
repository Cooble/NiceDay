#pragma once
#include "ndpch.h"
#include "ChunkMeshInstance.h"
#include "entity/Camera.h"
#include "BlockRegistry.h"
#include "graphics/buffer/FrameBuffer.h"


const int BLOCK_PIXEL_SIZE = 32;
struct StructChunkID
{
	union
	{
		struct
		{
			uint16_t x, y;
		};
		int id;
	};
	StructChunkID(int idd):id(idd){}
	StructChunkID(uint16_t xx, uint16_t yy):x(xx),y(yy){}
};
typedef float half;//maybe in future replace with half float to save space

class WorldRenderManager
{
private:
	FrameBuffer* m_light_frame_buffer;
	
	Texture* m_light_texture;
	Texture* m_light_simple_texture;

	Program* m_light_program;
	Program* m_light_simple_program;

	VertexBuffer* m_light_VBO;
	VertexBuffer* m_light_simple_VBO;

	VertexArray* m_light_VAO;
	VertexArray* m_light_simple_VAO;

	glm::mat4 m_world_view_matrix;


	std::vector<ChunkMeshInstance*> m_chunks;
	half* m_light_map;//for 1 block there are 4 lightPixels
	World* m_world;
	Camera* m_camera;
	glm::mat4 m_view_matrix;
	int m_chunk_width, m_chunk_height;
	int last_cx=-10000, last_cy=-10000;//the chunk from which light is computed

	std::unordered_map<int, int> m_offset_map;//chunkID,offsetInBuffer

	void computeLight();
	inline half& lightValue(int x, int y) { return m_light_map[y*m_chunk_width*WORLD_CHUNK_SIZE+x]; }
	inline void clearLightMap() { memset(m_light_map, 0, getChunksSize()*WORLD_CHUNK_AREA*sizeof(half)); }
	inline float getBlockOpacity(int x, int y)
	{

		auto b = m_world->getBlockPointer(x, y);
		if (!b)
			return 1;
		return BlockRegistry::get().getBlock(b->id).getOpacity(*b);
	}

	int getChunkIndex(int cx, int cy);
public:
	WorldRenderManager(Camera* cam,World* world);
	~WorldRenderManager();
	void onScreenResize();
	void onUpdate();
	void render();
	void renderLightMap();
	void renderMainLightMap();
	inline int getChunksSize() const { return m_chunk_width * m_chunk_height; }
	inline std::unordered_map<int, int>& getMap() { return m_offset_map; }
	inline const glm::mat4& getWorldViewMatrix() { return m_world_view_matrix; }
};


