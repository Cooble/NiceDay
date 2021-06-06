#pragma once
#include "ndpch.h"
#include "ChunkMesh.h"
#include "world/Camera.h"
#include "LightCalculator.h"
#include "graphics/TestQuad.h"
#include "graphics/Effect.h"


namespace nd {
class BatchRenderer2D;
}

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
	nd::TestQuad* m_test_quad;
	nd::ShaderPtr m_sky_program;

	nd::FrameBufferTexturePair* m_fbo_pair;
	nd::GreenFilter* m_green_filter;
	nd::GaussianBlurMultiple* m_blur;
	nd::ScaleEdgesEffect* m_edge;
	nd::AlphaMaskEffect* m_block_mask;

	//background
	nd::FrameBuffer* m_bg_fbo;
	nd::FrameBuffer* m_bg_layer_fbo;

	//background
	nd::FrameBuffer* m_bg_sky_fbo;


	//light stuff
	LightCalculator& m_light_calculator;

	nd::FrameBuffer* m_light_fbo;
	//FrameBuffer* m_light_smooth_fbo;
	nd::FrameBuffer* m_block_fbo;
	nd::FrameBuffer* m_wall_fbo;
	nd::FrameBuffer* m_sky_fbo;
	nd::FrameBuffer* m_entity_fbo;


	//FrameBuffer* m_light_FBO;
	
	//Texture* m_light_texture;
	nd::Texture* m_light_simple_texture;
	nd::Texture* m_light_sky_simple_texture;
	nd::Texture* m_block_mask_texture;

	nd::ShaderPtr m_light_program;
	nd::ShaderPtr m_light_simple_program;

	nd::VertexBuffer* m_light_VBO;
	nd::VertexArray* m_light_VAO;
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
	void renderBiomeBackgroundToFBO(nd::BatchRenderer2D& batchRenderer);
public:
	
	WorldRenderManager(Camera* cam,World* world);
	~WorldRenderManager();
	void onScreenResize();
	// when new chunk is loaded or unloaded call this
	void refreshChunkList();
	//todo make this on different thread
	void update();
	void render(nd::BatchRenderer2D& batchRenderer, nd::FrameBuffer* fbo);
	void renderLightMap();
	void applyLightMap(const nd::Texture* lightmap, nd::FrameBuffer* fbo);
	int getChunksSize() const { return m_chunk_width * m_chunk_height; }
	std::unordered_map<int, int>& getMap() { return m_offset_map; }
	const glm::mat4& getProjMatrix() { return m_proj_matrix; }

	//const Texture* getLightTextureSmooth() { return m_light_smooth_fbo->getAttachment(0); }
	//Texture* getLightTextureBlur() { return m_blur->getTexture(); }
	
	const nd::Texture* getLightTextureBlur() { return m_light_fbo->getAttachment(1); }
	
	const nd::Texture* getLightTextureHard() { return m_light_fbo->getAttachment(0); }
	auto* getEntityFBO() { return m_entity_fbo; }
};


