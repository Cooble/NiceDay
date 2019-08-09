#pragma once
#include "ndpch.h"
#include "ChunkMesh.h"
#include "entity/Camera.h"
#include "graphics/buffer/FrameBuffer.h"
#include "LightCalculator.h"
#include "graphics/Sprite2D.h"
#include "graphics/TestQuad.h"
#include "graphics/Effect.h"


struct BiomeDistances
{
	int biomes[4];
	float intensities[4];

	BiomeDistances()
	{
		biomes[0] = -1;
		biomes[1] = -1;
		biomes[2] = -1;
		biomes[3] = -1;
	}
};
const int BLOCK_PIXEL_SIZE = 16;//todo this is fishy blocks should be 16pixels in size on screen and 8pixels on texture

class WorldRenderManager
{
private:
	TestQuad* m_test_quad;
	Shader* m_sky_program;

	FrameBufferTexturePair* m_fbo_pair;
	GreenFilter* m_green_filter;
	GaussianBlurMultiple* m_blur;
	ScaleEdgesEffect* m_edge;
	AlphaMaskEffect* m_block_mask;

	//whole_screen_quad
	VertexArray* m_full_screen_quad_VAO;
	VertexBuffer* m_full_screen_quad_VBO;

	//background
	FrameBufferTexturePair* m_bg_fbo;
	FrameBufferTexturePair* m_bg_layer_fbo;



	//light stuff
	LightCalculator& m_light_calculator;
	
	FrameBufferTexturePair* m_light_fbo;
	FrameBufferTexturePair* m_light_smooth_fbo;
	FrameBufferTexturePair* m_block_fbo;
	FrameBufferTexturePair* m_wall_fbo;

	//FrameBuffer* m_light_FBO;
	
	//Texture* m_light_texture;
	Texture* m_light_simple_texture;
	Texture* m_light_sky_simple_texture;
	Texture* m_block_mask_texture;

	Shader* m_light_program;
	Shader* m_light_simple_program;

	VertexBuffer* m_light_VBO;
	VertexArray* m_light_VAO;
	//end of light

	//converts from camera space to screen space (-1,-1,1,1)
	glm::mat4 m_proj_matrix;

	std::vector<ChunkMeshInstance*> m_chunks;

	World* m_world;
	Camera* m_camera;
	int m_chunk_width, m_chunk_height;
	int last_cx=-10000, last_cy=-10000;//the chunk from which light is computed

	std::unordered_map<int, int> m_offset_map;//chunkID,offsetInBuffer

	ChunkPos lightOffset;
	int getChunkIndex(int cx, int cy);
	glm::vec4 getSkyColor(float y);
	void renderBiomeBackgroundToFBO();
public:
	
	WorldRenderManager(Camera* cam,World* world);
	~WorldRenderManager();
	void onScreenResize();
	void onUpdate();
	void render();
	void renderLightMap();
	void applyLightMap(Texture* lightmap);
	inline int getChunksSize() const { return m_chunk_width * m_chunk_height; }
	inline std::unordered_map<int, int>& getMap() { return m_offset_map; }
	inline const glm::mat4& getProjMatrix() { return m_proj_matrix; }
};


