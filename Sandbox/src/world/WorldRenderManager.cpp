#include "ndpch.h"
#include "WorldRenderManager.h"
#include "App.h"
#include "Stats.h"
#include "graphics/Renderer.h"
#include "world/ChunkMesh.h"


#include "platform/OpenGL/GLRenderer.h"
#include "biome/BiomeRegistry.h"
#include <algorithm>
#include "graphics/GContext.h"
#include "graphics/Sprite2D.h"



WorldRenderManager::WorldRenderManager(Camera* cam, World* world)
	: m_light_calculator(world->getLightCalculator()),
	  m_light_fbo(nullptr),
	  m_bg_fbo(nullptr),
	  m_bg_layer_fbo(nullptr),
	  m_light_smooth_fbo(nullptr),
	  m_block_fbo(nullptr),
	  m_wall_fbo(nullptr),
	  m_block_mask_texture(nullptr),
	  m_light_simple_texture(nullptr),
	  m_light_sky_simple_texture(nullptr),
	  m_green_filter(nullptr),
	  m_blur(nullptr),
	  m_edge(nullptr),
	  m_block_mask(nullptr),
	  m_fbo_pair(nullptr),
	  m_world(world),
	  m_camera(cam)
{
	//test quad

	m_test_quad = new TestQuad();

	//setup sky
	m_sky_program = new Shader("res/shaders/Sky.shader");

	//setup bg
	m_bg_layer_fbo = new FrameBufferTexturePair();
	m_bg_fbo = new FrameBufferTexturePair();
	//setup main light texture
	m_light_fbo = new FrameBufferTexturePair();
	m_light_smooth_fbo = new FrameBufferTexturePair();
	m_block_fbo = new FrameBufferTexturePair();
	m_wall_fbo = new FrameBufferTexturePair();
	m_light_program = new Shader("res/shaders/Light.shader");
	m_light_program->bind();
	m_light_program->setUniform1i("u_texture", 0);
	m_light_program->unbind();
	float quad[] = {
		1, 0,
		1, 1,
		0, 0,
		0, 1,
	};
	m_light_VBO = VertexBuffer::create(quad, sizeof(quad));
	VertexBufferLayout l;
	l.push<float>(2);
	m_light_VAO = VertexArray::create();
	m_light_VBO->setLayout(l);
	m_light_VAO->addBuffer(*m_light_VBO);

	//setup simple light texture
	m_light_simple_program = new Shader("res/shaders/LightTexturePiece.shader");
	m_light_simple_program->bind();
	m_light_simple_program->setUniform1i("u_texture_0", 0);
	m_light_simple_program->setUniform1i("u_texture_1", 1);
	m_light_simple_program->unbind();
	float simpleQuad[] = {
		1, -1,
		1, 1,
		-1, -1,
		-1, 1,

	};
	m_full_screen_quad_VBO = VertexBuffer::create(simpleQuad, sizeof(simpleQuad));
	m_full_screen_quad_VBO->setLayout(l);
	m_full_screen_quad_VAO = VertexArray::create();

	m_full_screen_quad_VAO->addBuffer(*m_full_screen_quad_VBO);

	m_fbo_pair = new FrameBufferTexturePair();
	onScreenResize();
}

WorldRenderManager::~WorldRenderManager()
{
	for (ChunkMeshInstance* m : m_chunks)
		delete m;

	delete m_bg_layer_fbo;
	delete m_bg_fbo;

	delete m_light_fbo;
	delete m_light_smooth_fbo;
	delete m_block_fbo;
	delete m_wall_fbo;

	delete m_light_program;
	delete m_light_simple_program;

	delete m_light_simple_texture;
	delete m_light_sky_simple_texture;
	delete m_block_mask_texture;

	delete m_light_VAO;
	delete m_full_screen_quad_VAO;

	delete m_light_VBO;
	delete m_full_screen_quad_VBO;

	delete m_green_filter;
	delete m_blur;
	delete m_edge;
	delete m_block_mask;

	delete m_fbo_pair;
};


void WorldRenderManager::onScreenResize()
{
	//build world view matrix
	float ratioo = (float)App::get().getWindow()->getHeight() / (float)App::get().getWindow()->getWidth();
	float chunkheightt = 2 * (float)BLOCK_PIXEL_SIZE / (float)App::get().getWindow()->getHeight();
	//this 2 means view is from -1 to 1 and not from 0 to 1
	m_proj_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(ratioo * chunkheightt, chunkheightt, 1));

	//refresh number of chunks 
	int screenWidth = App::get().getWindow()->getWidth();
	int screenHeight = App::get().getWindow()->getHeight();
	float chunkwidth = ((float)screenWidth / (float)BLOCK_PIXEL_SIZE) / (float)WORLD_CHUNK_SIZE;
	float chunkheight = ((float)screenHeight / (float)BLOCK_PIXEL_SIZE) / (float)WORLD_CHUNK_SIZE;
	m_chunk_width = ceil(chunkwidth) + 2;
	m_chunk_height = ceil(chunkheight) + 2;

	m_light_calculator.setDimensions(m_chunk_width, m_chunk_height);

	m_chunks.reserve(getChunksSize());

	m_offset_map.clear();

	for (ChunkMeshInstance* m : m_chunks)
		m->m_enabled = false;

	while (m_chunks.size() < getChunksSize())
		m_chunks.push_back(new ChunkMeshInstance());

	//made default light texture with 1 channel
	if (m_light_simple_texture)
		delete m_light_simple_texture;
	if (m_light_sky_simple_texture)
		delete m_light_sky_simple_texture;

	TextureInfo info;

	//setup green
	info.size(screenWidth, screenHeight);
	info.wrapMode(TextureWrapMode::CLAMP_TO_EDGE);

	m_green_filter = new GreenFilter(info);

	m_fbo_pair->replaceTexture(Texture::create(info));

	info.size(screenWidth, screenHeight);


	info = TextureInfo();

	info.size(m_chunk_width * WORLD_CHUNK_SIZE, m_chunk_height * WORLD_CHUNK_SIZE).f_format = TextureFormat::RED;
	m_light_simple_texture = Texture::create(info);
	m_light_sky_simple_texture = Texture::create(info);

	//block mask txutre
	info = TextureInfo();
	info.size(screenWidth / 2, screenHeight / 2).filterMode(TextureFilterMode::NEAREST).format(TextureFormat::RED);

	if (m_block_mask_texture)
		delete m_block_mask_texture;
	m_block_mask_texture = Texture::create(info);


	//made main light map texture with 4 channels


	info = TextureInfo();
	info.size(m_chunk_width * WORLD_CHUNK_SIZE * 2, m_chunk_height * WORLD_CHUNK_SIZE * 2).
	     filterMode(TextureFilterMode::NEAREST).format(TextureFormat::RGBA).wrapMode(TextureWrapMode::CLAMP_TO_EDGE);

	m_light_fbo->replaceTexture(Texture::create(info));

	info.filterMode(TextureFilterMode::LINEAR);
	m_light_smooth_fbo->replaceTexture(Texture::create(info));

	if (m_edge == nullptr)
	{
		m_edge = new ScaleEdgesEffect(info);
	}
	else
		m_edge->replaceTexture(info);

	if (m_blur == nullptr)
	{
		m_blur = new GaussianBlurMultiple(info, {2});
	}
	else
		m_blur->replaceTexture(info);


	info = TextureInfo();
	info.size(screenWidth, screenHeight);
	if (m_block_mask == nullptr)
	{
		m_block_mask = new AlphaMaskEffect(info);
	}
	else
		m_block_mask->replaceTexture(info);
	m_block_fbo->replaceTexture(Texture::create(info));
	m_wall_fbo->replaceTexture(Texture::create(info));

	//bg
	info = TextureInfo();
	info.size(screenWidth / 2, screenHeight / 2).filterMode(TextureFilterMode::NEAREST).format(TextureFormat::RGBA);

	m_bg_layer_fbo->replaceTexture(Texture::create(info));
	m_bg_fbo->replaceTexture(Texture::create(info));

	last_cx = -1000;
	//onUpdate();
}

float minim(float f0, float f1)
{
	if (f0 < f1)
		return f0;
	return f1;
}

inline float narrowFunc(float f)
{
	return std::clamp((f - 0.5f) * 2 + 0.5f, 0.0f, 1.0f);
}

BiomeDistances calculateBiomeDistances(Camera* cam, World& w)
{
	int cx = (int)cam->getPosition().x >> WORLD_CHUNK_BIT_SIZE;
	int cy = (int)cam->getPosition().y >> WORLD_CHUNK_BIT_SIZE;
	std::unordered_map<int, float> map;
	for (int x = -1; x < 2; ++x)
	{
		for (int y = -1; y < 2; ++y)
		{
			const Chunk* c = w.getChunk(cx + x, cy + y);
			if (c == nullptr)
				continue;
			int biome = c->getBiome();

			float length = glm::length(
				cam->getPosition() - glm::vec2((cx + x + 0.5f) * WORLD_CHUNK_SIZE, (cy + y + 0.5f) * WORLD_CHUNK_SIZE));
			//todo this seems fishy
			if (length <= WORLD_CHUNK_SIZE) //we care only about close chunks radius of chunks is 
				map[biome] = minim(map.find(biome) != map.end() ? map[biome] : 100000, length / (WORLD_CHUNK_SIZE));
		}
	}
	std::unordered_map<int, float> newmap;
	for (std::pair<int, float> element : map)
		newmap[element.first] = 1 - narrowFunc(element.second);

	float normalizer = 0;
	for (std::pair<int, float> element : newmap)
		normalizer += element.second;

	BiomeDistances out;
	int index = 0;
	for (std::pair<int, float> element : newmap)
	{
		out.biomes[index] = element.first;
		out.intensities[index] = element.second / normalizer;
		index++;
		ASSERT(index != 4, "This shouldnot happen, only 4 chunks can be closest to the camera");
	}
	return out;
}

void WorldRenderManager::refreshChunkList()
{
	m_light_calculator.setChunkOffset(last_cx, last_cy);

	std::set<int> toRemoveList;
	std::set<int> toLoadList;

	for (auto& iterator : m_offset_map)
	{
		toRemoveList.insert(iterator.first); //get all loaded chunks
	}
	for (int x = 0; x < m_chunk_width; x++)
	{
		for (int y = 0; y < m_chunk_height; y++)
		{
			auto targetX = last_cx + x;
			auto targetY = last_cy + y;
			if (
				targetX < 0
				|| targetY < 0
				|| targetX >= (m_world->getInfo().chunk_width)
				|| targetY >= (m_world->getInfo().chunk_height))
				continue;
			half_int mid = {targetX, targetY};
			auto cc = m_world->getChunkM(targetX, targetY);
			if (cc == nullptr)
			{
				ND_WARN("Cannot render unloaded chunk {},{}", mid.x, mid.y);
				continue;
			}
			if (m_offset_map.find(mid) == m_offset_map.end())
				toLoadList.insert(mid);
			else if (cc->isDirty())
			{
				m_chunks.at(m_offset_map[mid])->updateMesh(*m_world, *cc);
				cc->markDirty(false);
			}
			toRemoveList.erase(mid);
		}
	}
	for (int removed : toRemoveList)
	{
		m_chunks.at(m_offset_map[removed])->m_enabled = false;
		m_offset_map.erase(removed);
	}
	int lastFreeChunk = 0;
	for (int loade : toLoadList)
	{
		half_int loaded = loade;
		bool foundFreeChunk = false;
		for (int i = lastFreeChunk; i < m_chunks.size(); i++)
		{
			ChunkMeshInstance& c = *m_chunks[i];
			if (!c.m_enabled)
			{
				lastFreeChunk = i + 1;
				c.m_enabled = true;
				c.getPos().x = loaded.x * WORLD_CHUNK_SIZE;
				c.getPos().y = loaded.y * WORLD_CHUNK_SIZE;

				auto chunk = m_world->getChunkM(loaded);
				chunk->markDirty(false);
				c.updateMesh(*m_world, *chunk);

				m_offset_map[loaded] = i;
				foundFreeChunk = true;
				break;
			}
		}
		ASSERT(foundFreeChunk, "This shouldnt happen"); //chunks meshes should have static number 
	}
}

void WorldRenderManager::onUpdate()
{
	int cx = floor(m_camera->getPosition().x / WORLD_CHUNK_SIZE) - floor(m_chunk_width / 2.0f);
	int cy = floor(m_camera->getPosition().y / WORLD_CHUNK_SIZE) - floor(m_chunk_height / 2.0f);

	if (cx != last_cx || cy != last_cy || m_world->hasChunkChanged())
	{
		last_cx = cx;
		last_cy = cy;
		refreshChunkList();
	}

	//std::vector<int> chunksToErase;
	for (auto& iterator : m_offset_map)
	{
		half_int id = iterator.first;
		auto c = m_world->getChunkM(id);
		if (c) {
			if (c->isDirty())
			{
				m_chunks[iterator.second]->updateMesh(*m_world, *c);
				c->markDirty(false);
			}
		}
		else
			ND_WARN("Worldrendermanager cannot find chunk");
		//else
		//	chunksToErase.push_back(id);
	}
	//for (int id : chunksToErase)
	//	m_offset_map.erase(m_offset_map.find(id));


	//if (Stats::light_enable)
	//	computeLight();moved to render "thread"
}

int WorldRenderManager::getChunkIndex(int cx, int cy)
{
	half_int id = {cx, cy};
	auto got = m_offset_map.find(id);
	if (got != m_offset_map.end())
	{
		return got->second;
	}
	return -1;
}

glm::vec4 WorldRenderManager::getSkyColor(float y)
{
	auto upColor = glm::vec4(0, 0, 0, 1);
	auto downColor = glm::vec4(0.1, 0.8f, 1, 1);
	auto blACK = glm::vec4(0, 0, 0, 1);
	downColor = glm::mix(blACK, downColor, m_world->getSkyLight().a);

	y -= m_world->getInfo().terrain_level;
	y /= m_world->getInfo().chunk_height * WORLD_CHUNK_SIZE - m_world->getInfo().terrain_level;
	y /= 2;

	return glm::mix(downColor, upColor, y + 0.2f);
}

void WorldRenderManager::renderBiomeBackgroundToFBO()
{
	using namespace glm;

	vec2 screenDim = vec2(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());
	vec2 lowerScreen = m_camera->getPosition() - ((screenDim / (float)BLOCK_PIXEL_SIZE));
	vec2 upperScreen = m_camera->getPosition() + ((screenDim / (float)BLOCK_PIXEL_SIZE));
	screenDim = upperScreen - lowerScreen;

	BiomeDistances distances = calculateBiomeDistances(m_camera, *m_world);
	//Stats::biome_distances = distances;

	m_bg_fbo->bind();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < 4; ++i)
	{
		int biome = distances.biomes[i];
		if (biome == -1)
			break;
		float intensity = distances.intensities[i];
		Biome& b = BiomeRegistry::get().getBiome(biome);
		b.update(m_world, m_camera);
		Sprite2D** sprites = b.getBGSprites();
		sprites[0]->getProgram().bind();
		sprites[0]->getVAO().bind();


		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_bg_layer_fbo->bind();
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		for (int i = 0; i < b.getBGSpritesSize(); i++)
		{
			Sprite2D& sprite = *sprites[i];
			sprite.getProgram().setUniformMat4("u_model_transform", sprite.getModelMatrix());
			sprite.getProgram().setUniformMat4("u_uv_transform", sprite.getUVMatrix());

			sprite.getTexture().bind(0);
			GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
		}
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
		glBlendColor(0, 0, 0, intensity);

		m_bg_fbo->bind();

		Sprite2D::getProgramStatic().bind();
		Sprite2D::getVAOStatic().bind();
		auto m = glm::translate(glm::mat4(1.0f), glm::vec3(-1, -1, 0));
		m = glm::scale(m, glm::vec3(2, 2, 0));
		Sprite2D::getProgramStatic().setUniformMat4("u_model_transform", m);
		Sprite2D::getProgramStatic().setUniformMat4("u_uv_transform", glm::mat4(1.0f));
		m_bg_layer_fbo->getTexture()->bind(0);
		GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
	}

	m_bg_fbo->unbind();
	glDisable(GL_BLEND);
}

void WorldRenderManager::render()
{
	//light
	if (Stats::light_enable)
		renderLightMap();

	//chunk render
	auto& chunkProgram = *ChunkMesh::getProgram();

	//bg
	renderBiomeBackgroundToFBO();

	Gcon.setClearColor(0, 0, 0, 0);

	m_wall_fbo->bind();
	{
		Gcon.disableBlend();
		Gcon.clear(COLOR_BUFFER_BIT);

		//sky render
		float CURSOR_Y = App::get().getWindow()->getHeight() / BLOCK_PIXEL_SIZE + m_camera->getPosition().y;
		float CURSOR_YY = -(float)App::get().getWindow()->getHeight() / BLOCK_PIXEL_SIZE + m_camera->getPosition().y;
		m_sky_program->bind();
		auto upColor = getSkyColor(CURSOR_Y);
		auto downColor = getSkyColor(CURSOR_YY);
		m_sky_program->setUniform4f("u_up_color", upColor.r, upColor.g, upColor.b, upColor.a);
		m_sky_program->setUniform4f("u_down_color", downColor.r, downColor.g, downColor.b, downColor.a);
		m_full_screen_quad_VAO->bind();
		GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

		//background
		Gcon.enableBlend();
		Gcon.setBlendFuncSeparate(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA, Blend::ZERO, Blend::ONE);


		Sprite2D::getProgramStatic().bind();
		Sprite2D::getVAOStatic().bind();
		auto m = glm::translate(glm::mat4(1.0f), glm::vec3(-1, -1, 0));
		m = glm::scale(m, glm::vec3(2, 2, 0));
		Sprite2D::getProgramStatic().setUniformMat4("u_model_transform", m);
		Sprite2D::getProgramStatic().setUniformMat4("u_uv_transform", glm::mat4(1.0f));
		m_bg_fbo->getTexture()->bind(0);
		GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

		//walls
		chunkProgram.bind();
		chunkProgram.setUniform1i("u_texture_atlas_width", WALL_TEXTURE_ATLAS_SIZE);
		chunkProgram.setUniform1i("u_corner_atlas_width", WALL_CORNER_ATLAS_SIZE);
		ChunkMesh::getAtlas()->bind(0);
		ChunkMesh::getCornerAtlas()->bind(1);

		for (ChunkMeshInstance* mesh : m_chunks)
		{
			if (!(mesh->m_enabled))
				continue;

			auto worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(
				                                  mesh->getPos().x - m_camera->getPosition().x,
				                                  mesh->getPos().y - m_camera->getPosition().y, 0.0f));
			worldMatrix = glm::scale(worldMatrix, glm::vec3(0.5f, 0.5f, 1));
			chunkProgram.setUniformMat4("u_transform", getProjMatrix() * worldMatrix);

			mesh->getWallVAO().bind();
			GLCall(glDrawArrays(GL_POINTS, 0, WORLD_CHUNK_AREA * 4));
		}
		m_wall_fbo->unbind();

		if (Stats::light_enable)
		{
			m_edge->render(m_light_fbo->getTexture(), Stats::edge_scale);
			m_blur->render(m_edge->getTexture());
			Gcon.enableBlend();
			m_wall_fbo->bind();
			applyLightMap(m_blur->getTexture());
		}
	}


	//blocks
	m_block_fbo->bind();
	{
		Gcon.clear(COLOR_BUFFER_BIT);
		Gcon.enableBlend();
		Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
		chunkProgram.bind();
		ChunkMesh::getAtlas()->bind(0);
		ChunkMesh::getCornerAtlas()->bind(1);
		chunkProgram.setUniform1i("u_texture_atlas_width", BLOCK_TEXTURE_ATLAS_SIZE);
		chunkProgram.setUniform1i("u_corner_atlas_width", BLOCK_CORNER_ATLAS_SIZE);
		for (ChunkMeshInstance* mesh : m_chunks)
		{
			if (!(mesh->m_enabled))
				continue;

			auto world_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(
				                                   mesh->getPos().x - m_camera->getPosition().x,
				                                   mesh->getPos().y - m_camera->getPosition().y, 0.0f));
			chunkProgram.setUniformMat4("u_transform", getProjMatrix() * world_matrix);

			mesh->getVAO().bind();
			GLCall(glDrawArrays(GL_POINTS, 0, WORLD_CHUNK_AREA));
		}
		if (Stats::light_enable)
			applyLightMap(m_light_fbo->getTexture());
	}
	m_block_fbo->unbind();


	Gcon.enableBlend();
	Gcon.setBlendFuncSeparate(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA, Blend::ZERO, Blend::ONE);

	Effect::renderToScreen(m_wall_fbo->getTexture());
	Effect::renderToScreen(m_block_fbo->getTexture());
}

void WorldRenderManager::renderLightMap()
{
	if (m_light_calculator.isFreshMap())
	{
		m_light_simple_texture->setPixels(m_light_calculator.getCurrentLightMap());
		m_light_sky_simple_texture->setPixels(m_light_calculator.getCurrentLightMapChunkBack());
		lightOffset = m_light_calculator.getCurrentOffset();
	}

	Gcon.disableBlend();
	Gcon.setClearColor(1, 1, 1, 0);

	m_light_simple_program->bind();
	m_light_simple_program->setUniformVec4f("u_chunkback_color", m_world->getSkyLight());
	m_light_simple_texture->bind(0);
	m_light_sky_simple_texture->bind(1);
	m_full_screen_quad_VAO->bind();

	m_light_fbo->bind();
	{
		Gcon.clear(COLOR_BUFFER_BIT);
		GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
	}
	m_light_smooth_fbo->bind();
	{
		Gcon.clear(COLOR_BUFFER_BIT);
		GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
	}
	m_light_smooth_fbo->unbind();
	m_blur->render(m_light_smooth_fbo->getTexture());
}

void WorldRenderManager::applyLightMap(Texture* lightmap)
{
	auto worldMatrix = glm::mat4(1.0f);
	worldMatrix = glm::translate(worldMatrix, glm::vec3(
		                             lightOffset.first * WORLD_CHUNK_SIZE - m_camera->getPosition().x,
		                             lightOffset.second * WORLD_CHUNK_SIZE - m_camera->getPosition().y, 0.0f));
	worldMatrix = glm::scale(worldMatrix,
	                         glm::vec3(m_chunk_width * WORLD_CHUNK_SIZE, m_chunk_height * WORLD_CHUNK_SIZE, 1));

	lightmap->bind(0);

	m_light_program->bind();
	m_light_program->setUniformMat4("u_transform", getProjMatrix() * worldMatrix);

	m_light_VAO->bind();

	glViewport(0, 0, App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA,GL_ZERO,GL_ONE);
	GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
	glDisable(GL_BLEND);
}
