#include "ndpch.h"
#include "WorldRenderManager.h"
#include "core/App.h"
#include "core/Stats.h"
#include "graphics/Renderer.h"
#include "graphics/BatchRenderer2D.h"
#include "world/ChunkMesh.h"


#include "platform/OpenGL/GLRenderer.h"
#include "biome/BiomeRegistry.h"
#include <algorithm>
#include "graphics/GContext.h"
#include "graphics/Sprite2D.h"
#include "graphics/Renderable2D.h"
#include "GLFW/glfw3.h"
#include "core/ImGuiLayer.h"

WorldRenderManager::WorldRenderManager(Camera* cam, World* world)
	: m_light_calculator(world->getLightCalculator()),
	m_light_fbo(nullptr),
	m_bg_fbo(nullptr),
	m_bg_layer_fbo(nullptr),
	m_bg_sky_fbo(nullptr),
	//m_light_smooth_fbo(nullptr),
	m_block_fbo(nullptr),
	m_entity_fbo(nullptr),
	m_wall_fbo(nullptr),
	m_sky_fbo(nullptr),
	m_block_mask_texture(nullptr),
	m_light_simple_texture(nullptr),
	m_light_sky_simple_texture(nullptr),
	m_green_filter(nullptr),
	m_blur(nullptr),
	m_edge(nullptr),
	m_block_mask(nullptr),
	m_fbo_pair(nullptr),
	m_world(world),
	m_camera(cam),
	m_chunks(0)
{
	//test quad

	m_test_quad = new TestQuad();

	//setup sky
	m_sky_program = ShaderLib::loadOrGetShader("res/shaders/Sky.shader");


	//=============prepare FBOs without specifiying dimensions=================
	
	//setup bg
	auto bgInfo = TextureInfo().
		filterMode(TextureFilterMode::NEAREST).
		format(TextureFormat::RGBA);
	m_bg_layer_fbo = FrameBuffer::create(bgInfo);
	m_bg_fbo = FrameBuffer::create(bgInfo);
	m_bg_sky_fbo = FrameBuffer::create(bgInfo);
	
	//setup main light texture

	auto lightInfo = TextureInfo().
		filterMode(TextureFilterMode::NEAREST).
		format(TextureFormat::RGBA).
		wrapMode(TextureWrapMode::CLAMP_TO_EDGE);
	auto lightInfoSmooth = lightInfo;
	lightInfoSmooth.filterMode(TextureFilterMode::LINEAR);
	
	FrameBufferInfo finfo;
	finfo.type = FBType::NORMAL_TARGET;
	finfo.textureInfoCount = 2;
	TextureInfo infos[2]{ lightInfo ,lightInfoSmooth};
	finfo.textureInfos = infos;
	
	m_light_fbo = FrameBuffer::create(finfo);
	
	//lightInfo.filterMode(TextureFilterMode::LINEAR);
	//m_light_smooth_fbo = FrameBuffer::create(lightInfo);

	//other
	TextureInfo defaultInfo;
	m_entity_fbo = FrameBuffer::create(defaultInfo);
	m_block_fbo = FrameBuffer::create(defaultInfo);
	m_wall_fbo = FrameBuffer::create(defaultInfo);
	m_sky_fbo = FrameBuffer::create(defaultInfo);
	

	//============OTHER==================
	m_light_program = ShaderLib::loadOrGetShader("res/shaders/Light.shader");
	m_light_program->bind();
	dynamic_cast<GLShader*>(m_light_program)->setUniform1i("u_texture", 0);
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
	m_light_simple_program = ShaderLib::loadOrGetShader("res/shaders/LightTexturePiece.shader");
	m_light_simple_program->bind();
	dynamic_cast<GLShader*>(m_light_simple_program)->setUniform1i("u_texture_0", 0);
	dynamic_cast<GLShader*>(m_light_simple_program)->setUniform1i("u_texture_1", 1);
	m_light_simple_program->unbind();

	m_fbo_pair = new FrameBufferTexturePair();
	onScreenResize();
}

WorldRenderManager::~WorldRenderManager()
{
	delete m_bg_layer_fbo;
	delete m_bg_fbo;
	delete m_bg_sky_fbo;

	delete m_light_fbo;
	//delete m_light_smooth_fbo;
	delete m_block_fbo;
	delete m_wall_fbo;
	delete m_sky_fbo;

	//delete m_light_program;
	//delete m_light_simple_program;

	delete m_light_simple_texture;
	delete m_light_sky_simple_texture;
	delete m_block_mask_texture;

	delete m_light_VAO;

	delete m_light_VBO;

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
	m_chunk_width = ceil(chunkwidth) + 3;
	m_chunk_height = ceil(chunkheight) + 3;

	//todo fix bug with light borders
	/*m_light_chunk_width = m_chunk_width + 2;
	m_light_chunk_height = m_chunk_width + 2;*/
	m_light_calculator.setDimensions(m_chunk_width, m_chunk_height);

	m_offset_map.clear();

	m_chunks.reserve(getChunksSize());
	for (auto chunk : m_chunks.getChunks())
	{
		chunk->enabled = false;
	}

	

	TextureInfo info;

	//setup green
	info.size(screenWidth, screenHeight);
	info.wrapMode(TextureWrapMode::CLAMP_TO_EDGE);

	m_green_filter = new GreenFilter(info);

	m_fbo_pair->replaceTexture(Texture::create(info));

	info.size(screenWidth, screenHeight);


	//mono channel light texture
	if (m_light_simple_texture)
		delete m_light_simple_texture;
	if (m_light_sky_simple_texture)
		delete m_light_sky_simple_texture;
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


	info = TextureInfo();
	info.size(screenWidth, screenHeight);
	if (m_block_mask == nullptr)
	{
		m_block_mask = new AlphaMaskEffect(info);
	}
	else
		m_block_mask->replaceTexture(info);


	//light
	//m_light_smooth_fbo->resize(m_chunk_width * WORLD_CHUNK_SIZE * 2, m_chunk_height * WORLD_CHUNK_SIZE * 2);
	m_light_fbo->resize(m_chunk_width * WORLD_CHUNK_SIZE * 2, m_chunk_height * WORLD_CHUNK_SIZE * 2);

	//bg
	m_bg_layer_fbo->resize(screenWidth, screenHeight);
	m_bg_sky_fbo->resize(screenWidth, screenHeight);
	m_bg_fbo->resize(screenWidth, screenHeight);

	//other
	m_entity_fbo->resize(screenWidth, screenHeight);
	m_block_fbo->resize(screenWidth, screenHeight);
	m_wall_fbo->resize(screenWidth, screenHeight);
	m_sky_fbo->resize(screenWidth, screenHeight);


	if (m_edge == nullptr)
	{
		m_edge = new ScaleEdgesEffect(info);
	}
	else
		m_edge->replaceTexture(info);

	if (m_blur == nullptr)
	{
		//m_blur = new GaussianBlurMultiple(info, {2});
		m_blur = new GaussianBlurMultiple(info, { 2 });
	}
	else
		m_blur->replaceTexture(info);
	
	last_cx = -1000;
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
		toRemoveList.insert(iterator.first); //get all loaded chunks

	for (int x = 0; x < m_chunk_width; x++)
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
			half_int mid = { targetX, targetY };
			auto cc = m_world->getChunkM(targetX, targetY);
			if (cc == nullptr)
			{
				//ND_WARN("Cannot render unloaded chunk {},{}", mid.x, mid.y);
				continue;
			}
			if (m_offset_map.find(mid) == m_offset_map.end())
				toLoadList.insert(mid);
			else if (cc->isDirty())
			{
				m_chunks.getChunks().at(m_offset_map[mid])->updateMesh(*m_world, *cc);
				cc->markDirty(false);
			}
			toRemoveList.erase(mid);
		}
	for (int removed : toRemoveList)
	{
		m_chunks.getChunks().at(m_offset_map[removed])->enabled = false;
		m_offset_map.erase(removed);
	}
	int lastFreeChunk = 0;
	for (int loade : toLoadList)
	{
		half_int loaded = loade;
		bool foundFreeChunk = false;
		auto& c = *m_chunks.getFreeChunk();
		c.getPos().x = loaded.x * WORLD_CHUNK_SIZE;
		c.getPos().y = loaded.y * WORLD_CHUNK_SIZE;

		auto chunk = m_world->getChunkM(loaded);
		chunk->markDirty(false);
		c.updateMesh(*m_world, *chunk);

		m_offset_map[loaded] = c.getIndex();
		foundFreeChunk = true;

		ASSERT(foundFreeChunk, "This shouldnt happen"); //chunks meshes should have static number 
	}
}

void WorldRenderManager::update()
{
	int cx = m_camera->getPosition().x / WORLD_CHUNK_SIZE - m_chunk_width / 2.0f;
	int cy = m_camera->getPosition().y / WORLD_CHUNK_SIZE - m_chunk_height / 2.0f;

	if (cx != last_cx || cy != last_cy || m_world->hasChunkChanged())
	{
		last_cx = cx;
		last_cy = cy;
		refreshChunkList();
	}

	for (auto& iterator : m_offset_map)
	{
		half_int id = iterator.first;
		auto c = m_world->getChunkM(id);
		if (c) {
			if (c->isDirty())
			{
				m_chunks.getChunks()[iterator.second]->updateMesh(*m_world, *c);
				c->markDirty(false);
			}
		}
		else
			ND_WARN("Worldrendermanager cannot find chunk");
	}

}

int WorldRenderManager::getChunkIndex(int cx, int cy)
{
	half_int id = { cx, cy };
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
	downColor = glm::mix(blACK, downColor, m_world->getSkyLight().a - 0.15f);

	y -= m_world->getInfo().terrain_level;
	y /= m_world->getInfo().chunk_height * WORLD_CHUNK_SIZE - m_world->getInfo().terrain_level;
	y /= 2;

	return glm::mix(downColor, upColor, y + 0.2f);
}

void WorldRenderManager::renderBiomeBackgroundToFBO(BatchRenderer2D& batchRenderer)
{
	ND_PROFILE_METHOD();
	using namespace glm;

	vec2 screenDim = vec2(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());
	vec2 lowerScreen = m_camera->getPosition() - ((screenDim / (float)BLOCK_PIXEL_SIZE));
	vec2 upperScreen = m_camera->getPosition() + ((screenDim / (float)BLOCK_PIXEL_SIZE));
	screenDim = upperScreen - lowerScreen;

	BiomeDistances distances = calculateBiomeDistances(m_camera, *m_world);
	//Stats::biome_distances = distances;

	Gcon.enableBlend();
	m_bg_fbo->bind();
	m_bg_fbo->clear(BuffBit::COLOR);

	for (int i = 0; i < 4; ++i)
	{
		int biome = distances.biomes[i];
		if (biome == -1)
			break;
		float intensity = distances.intensities[i];
		Biome& b = BiomeRegistry::get().getBiome(biome);
		b.update(m_world, m_camera);
		Sprite2D** sprites = b.getBGSprites();

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_bg_layer_fbo->bind();
		m_bg_layer_fbo->clear(BuffBit::COLOR);
		batchRenderer.begin(m_bg_layer_fbo);
		for (int i = 0; i < b.getBGSpritesSize(); i++)
		{
			Sprite2D& sprite = *sprites[i];

			glm::vec4 uv0 = sprite.getUVMatrix() * glm::vec4(0, 0, 0, 1);
			glm::vec4 uv1 = sprite.getUVMatrix() * glm::vec4(1, 0, 0, 1);
			glm::vec4 uv2 = sprite.getUVMatrix() * glm::vec4(1, 1, 0, 1);
			glm::vec4 uv3 = sprite.getUVMatrix() * glm::vec4(0, 1, 0, 1);
			UVQuad uv = UVQuad(
				glm::vec2(uv0.x, uv0.y),
				glm::vec2(uv1.x, uv1.y),
				glm::vec2(uv2.x, uv2.y),
				glm::vec2(uv3.x, uv3.y)
			);

			batchRenderer.push(sprite.getModelMatrix());
			batchRenderer.submitTextureQuad({ 0,0,0 }, { 1,1 }, uv, &sprite.getTexture(), sprite.getAlpha());
			batchRenderer.pop();

		}
		batchRenderer.flush();

		glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
		glBlendColor(0, 0, 0, intensity);

		Effect::render(m_bg_layer_fbo->getAttachment(),m_bg_fbo);
	}

	m_bg_fbo->unbind();


	//sky
	m_bg_sky_fbo->bind();
	m_bg_sky_fbo->clear(BuffBit::COLOR);
	for (int i = 0; i < 4; ++i)
	{
		int biome = distances.biomes[i];
		if (biome == -1)
			break;
		float intensity = distances.intensities[i];
		Biome& b = BiomeRegistry::get().getBiome(biome);
		Sprite2D** sprites = b.getSkyBGSprites();

		Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);

		m_bg_layer_fbo->bind();
		m_bg_layer_fbo->clear(BuffBit::COLOR);
		batchRenderer.begin(m_bg_layer_fbo);
		for (int i = 0; i < b.getSkyBGSpritesSize(); i++)
		{
			Sprite2D& sprite = *sprites[i];

			glm::vec4 uv0 = sprite.getUVMatrix() * glm::vec4(0, 0, 0, 1);
			glm::vec4 uv1 = sprite.getUVMatrix() * glm::vec4(1, 0, 0, 1);
			glm::vec4 uv2 = sprite.getUVMatrix() * glm::vec4(1, 1, 0, 1);
			glm::vec4 uv3 = sprite.getUVMatrix() * glm::vec4(0, 1, 0, 1);
			UVQuad uv = UVQuad(
				glm::vec2(uv0.x, uv0.y),
				glm::vec2(uv1.x, uv1.y),
				glm::vec2(uv2.x, uv2.y),
				glm::vec2(uv3.x, uv3.y)
			);

			batchRenderer.push(sprite.getModelMatrix());
			batchRenderer.submitTextureQuad({ 0,0,0 }, { 1,1 }, uv, &sprite.getTexture(), sprite.getAlpha());
			batchRenderer.pop();
		}
		batchRenderer.flush();

		glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
		glBlendColor(0, 0, 0, intensity);

		Effect::render(m_bg_layer_fbo->getAttachment(), m_bg_sky_fbo);
	}
	m_bg_sky_fbo->unbind();
}

void WorldRenderManager::render(BatchRenderer2D& batchRenderer, FrameBuffer* fbo)
{
	ND_PROFILE_METHOD();

	fbo->bind();


	//light
	if (Stats::light_enable)
		renderLightMap();
	//bg
	renderBiomeBackgroundToFBO(batchRenderer);

	//chunk render
	auto& chunkProgram = *ChunkMesh::getProgram();

		//walls
	m_wall_fbo->bind();
	{
		ND_PROFILE_SCOPE("walls render");
		m_wall_fbo->clear(BuffBit::COLOR);
		//background
		Gcon.enableBlend();
		Gcon.setBlendEquation(BlendEquation::FUNC_ADD);
		Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
		Effect::render(m_bg_fbo->getAttachment(), m_wall_fbo);

		m_wall_fbo->bind();
		Gcon.enableBlend();
		
		//walls
		chunkProgram.bind();
		ChunkMesh::getAtlas()->bind(0);
		ChunkMesh::getCornerAtlas()->bind(1);
		m_chunks.getVAO().bind();

		for (auto mesh : m_chunks.getChunks())
		{
			if (!(mesh->enabled))
				continue;

			auto worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(
				mesh->getPos().x - m_camera->getPosition().x,
				mesh->getPos().y - m_camera->getPosition().y, 0.0f));
			worldMatrix = glm::scale(worldMatrix, glm::vec3(0.5f, 0.5f, 1));
			dynamic_cast<GLShader*>(&chunkProgram)->setUniformMat4("u_transform", getProjMatrix() * worldMatrix);
			GLCall(glDrawArrays(GL_TRIANGLES, m_chunks.getVertexOffsetToWallBuffer(mesh->getIndex()), WORLD_CHUNK_AREA * 4 * 6));
		}
		m_wall_fbo->unbind();


		if (Stats::light_enable)
		{
			//m_edge->render(m_light_fbo->getAttachment(), Stats::edge_scale);
			//m_blur->render(m_edge->getTexture());
			//applyLightMap(m_light_smooth_fbo->getAttachment(), m_wall_fbo);
			applyLightMap(m_light_fbo->getAttachment(1), m_wall_fbo);
			//applyLightMap(m_blur->getTexture(),m_wall_fbo);
		}
	}
	m_wall_fbo->unbind();
	
	//blocks
	m_block_fbo->bind();
	{
		ND_PROFILE_SCOPE("blocks render");
		m_block_fbo->clear(BuffBit::COLOR);
		Gcon.enableBlend();
		Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
		chunkProgram.bind();
		ChunkMesh::getAtlas()->bind(0);
		ChunkMesh::getCornerAtlas()->bind(1);
		m_chunks.getVAO().bind();
		for (auto mesh : m_chunks.getChunks())
		{
			if (!mesh->enabled)
				continue;

			auto world_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(
				mesh->getPos().x - m_camera->getPosition().x,
				mesh->getPos().y - m_camera->getPosition().y, 0.0f));
			dynamic_cast<GLShader*>(&chunkProgram)->setUniformMat4("u_transform", getProjMatrix() * world_matrix);
			GLCall(glDrawArrays(GL_TRIANGLES, m_chunks.getVertexOffsetToBlockBuffer(mesh->getIndex()), WORLD_CHUNK_AREA * 6));
		}
		if (Stats::light_enable)
			applyLightMap(m_light_fbo->getAttachment(),m_block_fbo);
	}

	m_block_fbo->unbind();

	fbo->bind();
	fbo->clear(BuffBit::COLOR | BuffBit::DEPTH);
	//sky render
	Gcon.disableBlend();
	float CURSOR_Y = (float)App::get().getWindow()->getHeight() / BLOCK_PIXEL_SIZE + m_camera->getPosition().y;
	float CURSOR_YY = -(float)App::get().getWindow()->getHeight() / BLOCK_PIXEL_SIZE + m_camera->getPosition().y;

	m_sky_program->bind();
	dynamic_cast<GLShader*>(m_sky_program)->setUniformVec4f("u_up_color", getSkyColor(CURSOR_Y));
	dynamic_cast<GLShader*>(m_sky_program)->setUniformVec4f("u_down_color", getSkyColor(CURSOR_YY));
	Effect::renderDefaultVAO();

	ND_IMGUI_VIEW_PROXY("sky first", fbo->getAttachment());
	Gcon.enableBlend();
	Gcon.setBlendEquation(BlendEquation::FUNC_ADD);
	Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
	Effect::render(m_bg_sky_fbo->getAttachment(),fbo);
	ND_IMGUI_VIEW_PROXY("sky2 after", fbo->getAttachment());
	Gcon.enableBlend();
	Effect::render(m_wall_fbo->getAttachment(),fbo);
	ND_IMGUI_VIEW_PROXY("sky3", fbo->getAttachment());
	Gcon.enableBlend();
	Effect::render(m_block_fbo->getAttachment(),fbo);
	ND_IMGUI_VIEW_PROXY("sky4", fbo->getAttachment());
	Gcon.enableBlend();
	fbo->bind();
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

	m_light_simple_program->bind();
	dynamic_cast<GLShader*>(m_light_simple_program)->setUniformVec4f("u_chunkback_color", m_world->getSkyLight());
	m_light_simple_texture->bind(0);
	m_light_sky_simple_texture->bind(1);
	Effect::getDefaultVAO().bind();

	m_light_fbo->bind();
	GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	//m_light_smooth_fbo->bind();
	//m_light_smooth_fbo->clear(BuffBit::COLOR, { 1,1,1,0 });
	//GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	//m_light_smooth_fbo->unbind();
	//m_blur->render(m_light_fbo->getAttachment(1));
}

void WorldRenderManager::applyLightMap(const Texture* lightmap, FrameBuffer* fbo)
{
	fbo->bind();
	auto worldMatrix = glm::mat4(1.0f);
	worldMatrix = glm::translate(worldMatrix, glm::vec3(
		lightOffset.first * WORLD_CHUNK_SIZE - m_camera->getPosition().x,
		lightOffset.second * WORLD_CHUNK_SIZE - m_camera->getPosition().y, 0.0f));
	worldMatrix = glm::scale(worldMatrix,
		glm::vec3(m_chunk_width * WORLD_CHUNK_SIZE, m_chunk_height * WORLD_CHUNK_SIZE, 1));

	lightmap->bind(0);

	m_light_program->bind();
	dynamic_cast<GLShader*>(m_light_program)->setUniformMat4("u_transform", getProjMatrix() * worldMatrix);

	m_light_VAO->bind();

	Gcon.enableBlend();
	Gcon.setBlendEquation(BlendEquation::FUNC_ADD);
	Gcon.setBlendFuncSeparate(Blend::ZERO, Blend::SRC_ALPHA, Blend::ZERO, Blend::ONE);
	GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
	Gcon.disableBlend();
}
