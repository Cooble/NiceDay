#pragma once
#include "ndpch.h"
#include "ChunkMeshInstance.h"
#include "entity/Camera.h"
#include "BlockRegistry.h"
#include "graphics/buffer/FrameBuffer.h"
#include "LightCalculator.h"


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

class WorldRenderManager
{
private:
	Program* m_sky_program;


	//whole_screen_quad
	VertexArray* m_full_screen_quad_VAO;
	VertexBuffer* m_full_screen_quad_VBO;



	//light stuff
	LightCalculator& m_light_calculator;
	FrameBuffer* m_light_frame_buffer;
	
	Texture* m_light_texture;
	Texture* m_light_simple_texture;

	Program* m_light_program;
	Program* m_light_simple_program;

	VertexBuffer* m_light_VBO;
	VertexArray* m_light_VAO;
	//end of light

	glm::mat4 m_world_view_matrix;

	std::vector<ChunkMeshInstance*> m_chunks;

	World* m_world;
	Camera* m_camera;
	glm::mat4 m_view_matrix;
	int m_chunk_width, m_chunk_height;
	int last_cx=-10000, last_cy=-10000;//the chunk from which light is computed

	std::unordered_map<int, int> m_offset_map;//chunkID,offsetInBuffer

	ChunkPos lightOffset;
	int getChunkIndex(int cx, int cy);
	glm::vec4 getSkyColor(float y);
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


