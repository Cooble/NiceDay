#pragma once
#include "ndpch.h"
#include "ChunkMesh.h"
#include "world/Camera.h"
#include "LightCalculator.h"
#include "graphics/TestQuad.h"
#include "graphics/Effect.h"


class BatchRenderer2D;

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
const int BLOCK_PIXEL_SIZE = 16;

class WorldRenderManager
{
private:
	TestQuad* m_test_quad;
	ShaderPtr m_sky_program;

	FrameBufferTexturePair* m_fbo_pair;
	GreenFilter* m_green_filter;
	GaussianBlurMultiple* m_blur;
	ScaleEdgesEffect* m_edge;
	AlphaMaskEffect* m_block_mask;

	//background
	FrameBuffer* m_bg_fbo;
	FrameBuffer* m_bg_layer_fbo;

	//background
	FrameBuffer* m_bg_sky_fbo;


	//light stuff
	LightCalculator& m_light_calculator;
	
	FrameBuffer* m_light_fbo;
	//FrameBuffer* m_light_smooth_fbo;
	FrameBuffer* m_block_fbo;
	FrameBuffer* m_wall_fbo;
	FrameBuffer* m_sky_fbo;
	FrameBuffer* m_entity_fbo;


	//FrameBuffer* m_light_FBO;
	
	//Texture* m_light_texture;
	Texture* m_light_simple_texture;
	Texture* m_light_sky_simple_texture;
	Texture* m_block_mask_texture;

	ShaderPtr m_light_program;
	ShaderPtr m_light_simple_program;

	VertexBuffer* m_light_VBO;
	VertexArray* m_light_VAO;
	//end of light

	//converts from camera space to screen space (-1,-1,1,1)
	glm::mat4 m_proj_matrix;

	ChunkMeshes m_chunks;

	World* m_world;
	Camera* m_camera;
	int m_chunk_width, m_chunk_height;
	int last_cx=-10000, last_cy=-10000;//the chunk from which light is computed

	std::unordered_map<int, int> m_offset_map;//chunkID,offsetInBuffer

	ChunkPos lightOffset;
	int m_light_chunk_width;
	int m_light_chunk_height;
	int getChunkIndex(int cx, int cy);
	glm::vec4 getSkyColor(float y);
	void renderBiomeBackgroundToFBO(BatchRenderer2D& batchRenderer);
public:
	
	WorldRenderManager(Camera* cam,World* world);
	~WorldRenderManager();
	void onScreenResize();
	// when new chunk is loaded or unloaded call this
	void refreshChunkList();
	//todo make this on different thread
	void update();
	void render(BatchRenderer2D& batchRenderer,FrameBuffer* fbo);
	void renderLightMap();
	void applyLightMap(const Texture* lightmap,FrameBuffer* fbo);
	int getChunksSize() const { return m_chunk_width * m_chunk_height; }
	std::unordered_map<int, int>& getMap() { return m_offset_map; }
	const glm::mat4& getProjMatrix() { return m_proj_matrix; }

	//const Texture* getLightTextureSmooth() { return m_light_smooth_fbo->getAttachment(0); }
	//Texture* getLightTextureBlur() { return m_blur->getTexture(); }
	
	const Texture* getLightTextureBlur() { return m_light_fbo->getAttachment(1); }
	
	const Texture* getLightTextureHard() { return m_light_fbo->getAttachment(0); }
	auto* getEntityFBO() { return m_entity_fbo; }
};


