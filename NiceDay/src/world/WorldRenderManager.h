#pragma once
#include "ndpch.h"
#include "ChunkMeshInstance.h"
#include "entity/Camera.h"

const int BLOCK_PIXEL_SIZE = 64;
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

class WorldRenderManager
{
private:
	std::vector<ChunkMeshInstance*> m_chunks;
	World* m_world;
	Camera* m_camera;
	glm::mat4 m_view_matrix;
	int m_chunk_width, m_chunk_height;
	int last_cx=-10000, last_cy=-10000;

	std::unordered_map<int, int> m_offset_map;//chunkID,offsetInBuffer

	int getChunkIndex(int cx, int cy);
public:
	WorldRenderManager(Camera* cam,World* world);
	~WorldRenderManager();
	void onScreenResize();
	void onUpdate();
	void render();
	inline int getChunksSize() const { return m_chunk_width * m_chunk_height; }
	inline std::unordered_map<int, int>& getMap() { return m_offset_map; }
};
