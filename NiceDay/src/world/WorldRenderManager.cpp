#include "ndpch.h"
#include "WorldRenderManager.h"
#include "Game.h"
#include "Stats.h"
#include "graphics/Renderer.h"
#include "world/ChunkMesh.h"


#include "biome/BiomeForest.h"
#include "biome/biomes.h"


WorldRenderManager::WorldRenderManager(Camera* cam, World* world)
	: m_light_calculator(world->getLightCalculator()),
	m_light_texture(nullptr),
	m_light_simple_texture(nullptr),
	m_world(world),
	m_camera(cam)
{
	//setup sky
	m_sky_program = new Shader("res/shaders/Sky.shader");

	//setup bg
	m_bg_FBO = FrameBuffer::create();
	m_bg_layer_FBO = FrameBuffer::create();

	//setup main light texture
	m_light_frame_FBO = FrameBuffer::create();
	m_light_program = new Shader("res/shaders/Light.shader");
	m_light_program->bind();
	m_light_program->setUniform1i("u_texture", 0);
	m_light_program->unbind();
	float quad[] = {
		1,0,
		1,1,
		0,0,
		0,1,
	};
	m_light_VBO = VertexBuffer::create(quad, sizeof(quad),GL_STATIC_DRAW);
	VertexBufferLayout l;
	l.push<float>(2);
	m_light_VAO = VertexArray::create();
	m_light_VAO->addBuffer(*m_light_VBO, l);

	//setup simple light texture
	m_light_simple_program = new Shader("res/shaders/LightTexturePiece.shader");
	m_light_simple_program->bind();
	m_light_simple_program->setUniform1i("u_texture", 0);
	m_light_simple_program->unbind();
	float simpleQuad[] = {
		 1,-1,
		 1,	1,
		-1,-1,
		-1, 1,

	};
	m_full_screen_quad_VBO = VertexBuffer::create(simpleQuad, sizeof(simpleQuad), GL_STATIC_DRAW);
	m_full_screen_quad_VAO = VertexArray::create();
	m_full_screen_quad_VAO->addBuffer(*m_full_screen_quad_VBO, l);
	onScreenResize();
}

WorldRenderManager::~WorldRenderManager()
{

	for (ChunkMeshInstance* m : m_chunks)
		delete m;
	delete m_light_frame_FBO;

	delete m_light_program;
	delete m_light_simple_program;

	delete m_light_texture;
	delete m_light_simple_texture;

	delete m_light_VAO;
	delete m_full_screen_quad_VAO;

	delete m_light_VBO;
	delete m_full_screen_quad_VBO;
};

void WorldRenderManager::onScreenResize()
{

	//build world view matrix
	float ratioo = (float)Game::get().getWindow()->getHeight() / (float)Game::get().getWindow()->getWidth();
	float chunkheightt = (float)BLOCK_PIXEL_SIZE / (float)Game::get().getWindow()->getHeight();
	m_proj_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(ratioo*chunkheightt, chunkheightt, 1));

	//refresh number of chunks 
	int screenWidth = Game::get().getWindow()->getWidth();
	int screenHeight = Game::get().getWindow()->getHeight();
	float chunkwidth = ((float)screenWidth / (float)BLOCK_PIXEL_SIZE) / (float)WORLD_CHUNK_SIZE;
	float chunkheight = ((float)screenHeight / (float)BLOCK_PIXEL_SIZE) / (float)WORLD_CHUNK_SIZE;
	m_chunk_width = ceil(chunkwidth) + 3;
	m_chunk_height = ceil(chunkheight) + 3;

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
	TextureInfo info;
	info.size(m_chunk_width*WORLD_CHUNK_SIZE, m_chunk_height*WORLD_CHUNK_SIZE).f_format = GL_RED;
	m_light_simple_texture = new Texture(info);


	//made main light map texture with 4 channels
	if (m_light_texture)
		delete m_light_texture;

	info = TextureInfo();
	info.size(m_chunk_width*WORLD_CHUNK_SIZE * 2, m_chunk_height*WORLD_CHUNK_SIZE * 2).filterMode(GL_NEAREST).format(GL_RGBA);

	m_light_texture = new Texture(info);
	m_light_frame_FBO->bind();
	m_light_frame_FBO->attachTexture(m_light_texture->getID());
	m_light_frame_FBO->unbind();

	//bg
	info = TextureInfo();
	info.size(screenWidth / 2, screenHeight / 2).filterMode(GL_NEAREST).format(GL_RGBA);

	m_bg_texture = new Texture(info);
	m_bg_FBO->bind();
	m_bg_FBO->attachTexture(m_bg_texture->getID());
	m_bg_FBO->unbind();

	//bg2
	m_bg_layer_texture = new Texture(info);
	m_bg_layer_FBO->bind();
	m_bg_layer_FBO->attachTexture(m_bg_layer_texture->getID());
	m_bg_layer_FBO->unbind();

	last_cx = -1000;
	onUpdate();
}
#include <algorithm>

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

BiomeDistances calculateBiomeDistances(Camera* cam, World* w)
{
	int cx = (int)cam->getPosition().x >> WORLD_CHUNK_BIT_SIZE;
	int cy = (int)cam->getPosition().y >> WORLD_CHUNK_BIT_SIZE;
	std::unordered_map<int, float> map;
	for (int x = -1; x < 2; ++x)
	{
		for (int y = -1; y < 2; ++y)
		{
			const Chunk* c = w->getLoadedChunkPointer(cx + x, cy + y);
			if (c == nullptr)
				continue;
			int biome = c->getBiome();

			float length = glm::length(cam->getPosition() - glm::vec2((cx + x + 0.5f)*WORLD_CHUNK_SIZE, (cy + y + 0.5f)*WORLD_CHUNK_SIZE));//todo this seems fishy
			if (length <= WORLD_CHUNK_SIZE)//we care only about close chunks radius of chunks is 
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

void WorldRenderManager::onUpdate()
{
	auto cx = round(m_camera->getPosition().x / WORLD_CHUNK_SIZE) - m_chunk_width / 2;
	auto cy = round(m_camera->getPosition().y / WORLD_CHUNK_SIZE) - m_chunk_height / 2;

	if (cx != last_cx || cy != last_cy)
	{
		m_light_calculator.setChunkOffset(cx, cy);
		last_cx = cx;
		last_cy = cy;

		std::set<int> toRemoveList;
		std::set<int> toLoadList;

		for (auto iterator = m_offset_map.begin(); iterator != m_offset_map.end(); ++iterator) {
			toRemoveList.insert(iterator->first);//get all loaded chunks
		}
		for (int x = 0; x < m_chunk_width; x++) {
			for (int y = 0; y < m_chunk_height; y++)
			{
				auto targetX = cx + x;
				auto targetY = cy + y;
				if (
					targetX < 0
					|| targetY < 0
					|| targetX >= (m_world->getInfo().chunk_width)
					|| targetY >= (m_world->getInfo().chunk_height))
					continue;
				StructChunkID mid = { Chunk::getChunkIDFromChunkPos(targetX, targetY) };
				int index = m_world->getChunkIndex(targetX, targetY);
				if (index == -1)
				{
					ND_WARN("Cannot render unloaded chunk {},{}", mid.x, mid.y);
					toRemoveList.erase(mid.id);
					continue;
				}

				Chunk& cc = m_world->getChunk(targetX, targetY);
				if (m_offset_map.find(mid.id) == m_offset_map.end())
					toLoadList.insert(mid.id);
				else if (cc.isDirty())
				{
					m_chunks.at(m_offset_map[mid.id])->updateMesh(*m_world, cc);
					cc.markDirty(false);
				}
				toRemoveList.erase(mid.id);
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
			StructChunkID loaded = loade;
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

					c.updateMesh(*m_world, m_world->getChunk(loaded.x, loaded.y));
					

					m_offset_map[loaded.id] = i;
					foundFreeChunk = true;
					break;
				}
			}
			ASSERT(foundFreeChunk, "This shouldnt happen");//chunks meshes should have static number 
		}

	}
	else
	{
		for (auto iterator = m_offset_map.begin(); iterator != m_offset_map.end(); ++iterator) {
			StructChunkID id = iterator->first;
			Chunk& c = m_world->getChunk(id.x, id.y);
			if (c.isDirty())
			{
				m_chunks[iterator->second]->updateMesh(*m_world, c);
				c.markDirty(false);
			}
		}
	}

	//if (Stats::light_enable)
	//	computeLight();moved to render "thread"

}

int WorldRenderManager::getChunkIndex(int cx, int cy)
{
	StructChunkID id = { (uint16_t)cx, (uint16_t)cy };
	auto got = m_offset_map.find(id.id);
	if (got != m_offset_map.end()) {
		return got->second;
	}
	return -1;
}

glm::vec4 WorldRenderManager::getSkyColor(float y)
{
	auto upColor = glm::vec4(0, 0, 0, 1);
	auto downColor = glm::vec4(0, 0.5f, 1, 1);


	y -= m_world->getInfo().terrain_level;
	y /= m_world->getInfo().chunk_height * WORLD_CHUNK_SIZE - m_world->getInfo().terrain_level;
	y /= 2;

	return glm::mix(downColor, upColor, y + 0.2f);
}

void WorldRenderManager::renderBiomeBackgroundToFBO()
{

	//bg render
	Sprite2D::init();

	using namespace glm;

	vec2 screenDim = vec2(Game::get().getWindow()->getWidth(), Game::get().getWindow()->getHeight());
	vec2 lowerScreen = m_camera->getPosition() - ((screenDim / (float)BLOCK_PIXEL_SIZE) / 2.0f);
	vec2 upperScreen = m_camera->getPosition() + ((screenDim / (float)BLOCK_PIXEL_SIZE) / 2.0f);
	screenDim = upperScreen - lowerScreen;

	BiomeDistances distances = calculateBiomeDistances(m_camera, m_world);
	Stats::biome_distances = distances;

	m_bg_FBO->bind();
	glViewport(0, 0, m_bg_texture->getWidth(), m_bg_texture->getHeight());
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

		m_bg_layer_FBO->bind();
		glViewport(0, 0, m_bg_layer_texture->getWidth(), m_bg_layer_texture->getHeight());
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		for (int i = 0; i < b.getBGSpritesSize(); i++)
		{
			Sprite2D& sprite = *sprites[i];
			sprite.getProgram().setUniformMat4("u_model_transform", sprite.getModelMatrix());
			sprite.getProgram().setUniformMat4("u_uv_transform", sprite.getUVMatrix());

			sprite.getTexture().bind(0);
			Call(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
		}
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
		glBlendColor(0, 0, 0, intensity);

		m_bg_FBO->bind();
		glViewport(0, 0, m_bg_texture->getWidth(), m_bg_texture->getHeight());

		Sprite2D::getProgramStatic().bind();
		Sprite2D::getVAOStatic().bind();
		auto m = glm::translate(glm::mat4(1.0f), glm::vec3(-1, -1, 0));
		m = glm::scale(m, glm::vec3(2, 2, 0));
		Sprite2D::getProgramStatic().setUniformMat4("u_model_transform", m);
		Sprite2D::getProgramStatic().setUniformMat4("u_uv_transform", glm::mat4(1.0f));
		m_bg_layer_texture->bind(0);
		Call(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	}

	m_bg_FBO->unbind();
	glViewport(0, 0, Game::get().getWindow()->getWidth(), Game::get().getWindow()->getHeight());
	glDisable(GL_BLEND);
}

void WorldRenderManager::render()
{

	//sky render
	float CURSOR_Y = Game::get().getWindow()->getHeight() / BLOCK_PIXEL_SIZE + m_camera->getPosition().y;
	float CURSOR_YY = -(float)Game::get().getWindow()->getHeight() / BLOCK_PIXEL_SIZE + m_camera->getPosition().y;
	m_sky_program->bind();
	auto upColor = getSkyColor(CURSOR_Y);
	auto downColor = getSkyColor(CURSOR_YY);
	m_sky_program->setUniform4f("u_up_color", upColor.r, upColor.g, upColor.b, upColor.a);
	m_sky_program->setUniform4f("u_down_color", downColor.r, downColor.g, downColor.b, downColor.a);
	m_full_screen_quad_VAO->bind();
	Call(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));


	//background
	renderBiomeBackgroundToFBO();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Sprite2D::getProgramStatic().bind();
	Sprite2D::getVAOStatic().bind();
	auto m = glm::translate(glm::mat4(1.0f), glm::vec3(-1, -1, 0));
	m = glm::scale(m, glm::vec3(2, 2, 0));
	Sprite2D::getProgramStatic().setUniformMat4("u_model_transform", m);
	Sprite2D::getProgramStatic().setUniformMat4("u_uv_transform", glm::mat4(1.0f));
	m_bg_texture->bind(0);
	Call(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	//chunk render
	auto& program = *ChunkMesh::getProgram();
	program.bind();
	program.setUniform1i("u_texture_atlas_width", WALL_TEXTURE_ATLAS_SIZE);
	program.setUniform1i("u_corner_atlas_width", WALL_CORNER_ATLAS_SIZE);
	ChunkMesh::getAtlas()->bind(0);
	ChunkMesh::getCornerAtlas()->bind(1);
	//walls
	for (ChunkMeshInstance* mesh : m_chunks)
	{
		if (!(mesh->m_enabled))
			continue;

		auto worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(
			mesh->getPos().x - m_camera->getPosition().x,
			mesh->getPos().y - m_camera->getPosition().y, 0.0f));
		worldMatrix = glm::scale(worldMatrix, glm::vec3(0.5f,0.5f,1));
		program.setUniformMat4("u_transform", getProjMatrix()*worldMatrix);

		mesh->getWallVAO().bind();
		Call(glDrawArrays(GL_POINTS, 0, WORLD_CHUNK_AREA*4));

	}
	//blocks
	program.setUniform1i("u_texture_atlas_width", BLOCK_TEXTURE_ATLAS_SIZE);
	program.setUniform1i("u_corner_atlas_width", BLOCK_CORNER_ATLAS_SIZE);
	for (ChunkMeshInstance* mesh : m_chunks)
	{
		if (!(mesh->m_enabled))
			continue;

		auto worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(
			mesh->getPos().x - m_camera->getPosition().x,
			mesh->getPos().y - m_camera->getPosition().y, 0.0f));

		program.setUniformMat4("u_transform", getProjMatrix()*worldMatrix);

		mesh->getVAO().bind();
		Call(glDrawArrays(GL_POINTS, 0, WORLD_CHUNK_AREA));

	}
	glDisable(GL_BLEND);


	//light stuff
	if (!Stats::light_enable)
		return;

	renderLightMap();
	renderMainLightMap();

}

void WorldRenderManager::renderLightMap()
{
	if (m_light_calculator.isFreshMap()) {
		m_light_simple_texture->setPixels(m_light_calculator.getCurrentLightMap());
		lightOffset = m_light_calculator.getCurrentOffset();
	}

	m_light_frame_FBO->bind();
	{
		Call(glViewport(0, 0, m_chunk_width*WORLD_CHUNK_SIZE * 2, m_chunk_height*WORLD_CHUNK_SIZE * 2));
		Call(glClearColor(0.5f, 0.5f, 0.5f, 1));
		Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		m_light_simple_program->bind();
		m_light_simple_texture->bind(0);
		m_full_screen_quad_VAO->bind();
		Call(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
		Call(glViewport(0, 0, Game::get().getWindow()->getWidth(), Game::get().getWindow()->getHeight()));
	}
	m_light_frame_FBO->unbind();
}

void WorldRenderManager::renderMainLightMap()
{
	auto worldMatrix = glm::mat4(1.0f);
	worldMatrix = glm::translate(worldMatrix, glm::vec3(
		lightOffset.first*WORLD_CHUNK_SIZE - m_camera->getPosition().x,
		lightOffset.second*WORLD_CHUNK_SIZE - m_camera->getPosition().y, 0.0f));
	worldMatrix = glm::scale(worldMatrix, glm::vec3(m_chunk_width*WORLD_CHUNK_SIZE, m_chunk_height*WORLD_CHUNK_SIZE, 1));

	m_light_program->bind();
	m_light_program->setUniformMat4("u_transform", getProjMatrix() * worldMatrix);
	m_light_texture->bind(0);
	m_light_VAO->bind();

	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	Call(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
	glDisable(GL_BLEND);
	glDisable(GL_FRAMEBUFFER_SRGB);
}