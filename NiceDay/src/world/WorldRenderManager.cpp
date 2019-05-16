#include "ndpch.h"
#include "WorldRenderManager.h"
#include "Game.h"
#include "Stats.h"
#include "graphics/Renderer.h"
#include "world/ChunkMeshInstance.h"
#include "glm/gtx/io.hpp"




WorldRenderManager::WorldRenderManager(Camera* cam, World* world)
	: m_light_texture(nullptr),
	m_light_simple_texture(nullptr),
	m_light_map(nullptr),
	m_world(world),
	m_camera(cam)
{
	//setup main light texture
	m_light_frame_buffer = new FrameBuffer();
	m_light_program = new Program("res/shaders/Light.shader");
	m_light_program->bind();
	m_light_program->setUniform1i("u_texture", 0);
	m_light_program->unbind();
	float quad[] = {
		1,0,
		1,1,
		0,0,
		0,1,

	};
	m_light_VBO = new VertexBuffer(quad, sizeof(quad));//todo is sizeof valid?
	VertexBufferLayout l;
	l.push<float>(2);
	m_light_VAO = new VertexArray();
	m_light_VAO->addBuffer(*m_light_VBO, l);

	//setup simple light texture
	m_light_simple_program = new Program("res/shaders/LightTexturePiece.shader");
	m_light_simple_program->bind();
	m_light_simple_program->setUniform1i("u_texture", 0);
	m_light_simple_program->unbind();
	float simpleQuad[] = {
		 1,-1,
		 1,	1,
		-1,-1,
		-1, 1,

	};
	m_light_simple_VBO = new VertexBuffer(simpleQuad, sizeof(simpleQuad));
	m_light_simple_VAO = new VertexArray();
	m_light_simple_VAO->addBuffer(*m_light_simple_VBO, l);
	onScreenResize();
}

WorldRenderManager::~WorldRenderManager()
{
	for (ChunkMeshInstance* m : m_chunks)
		delete m;

	delete[] m_light_map;
	delete m_light_frame_buffer;

	delete m_light_program;
	delete m_light_simple_program;

	delete m_light_texture;
	delete m_light_simple_texture;

	delete m_light_VAO;
	delete m_light_simple_VAO;

	delete m_light_VBO;
	delete m_light_simple_VBO;
};

void WorldRenderManager::onScreenResize()
{

	//build world view matrix
	float ratioo = (float)Game::get().getWindow()->getHeight() / (float)Game::get().getWindow()->getWidth();
	float chunkheightt = (float)BLOCK_PIXEL_SIZE / (float)Game::get().getWindow()->getHeight();
	m_world_view_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(ratioo*chunkheightt, chunkheightt, 1));

	int screenWidth = Game::get().getWindow()->getWidth();
	int screenHeight = Game::get().getWindow()->getHeight();


	float chunkwidth = ((float)screenWidth / (float)BLOCK_PIXEL_SIZE) / (float)WORLD_CHUNK_SIZE;
	float chunkheight = ((float)screenHeight / (float)BLOCK_PIXEL_SIZE) / (float)WORLD_CHUNK_SIZE;

	m_chunk_width = ceil(chunkwidth) + 3;
	m_chunk_height = ceil(chunkheight) + 3;

	m_chunks.reserve(getChunksSize());

	m_offset_map.clear();

	for (ChunkMeshInstance* m : m_chunks)
		m->m_enabled = false;

	while (m_chunks.size() < getChunksSize())
		m_chunks.push_back(new ChunkMeshInstance());

	if (m_light_map)
		delete[] m_light_map;
	m_light_map = new half[getChunksSize()*WORLD_CHUNK_AREA];


	//made default light texture with 1 channel
	if (m_light_simple_texture)
		delete m_light_simple_texture;
	m_light_simple_texture = new Texture(m_chunk_width*WORLD_CHUNK_SIZE, m_chunk_height*WORLD_CHUNK_SIZE, GL_LINEAR, GL_REPEAT, GL_RED);


	//made main light map texture with 4 channels
	if (m_light_texture)
		delete m_light_texture;
	m_light_texture = new Texture(m_chunk_width*WORLD_CHUNK_SIZE * 2, m_chunk_height*WORLD_CHUNK_SIZE * 2, GL_NEAREST, GL_REPEAT, GL_RGBA);

	m_light_frame_buffer->bind();
	m_light_frame_buffer->attachTexture(m_light_texture->getID());
	m_light_frame_buffer->unbind();


	last_cx = -1000;
	onUpdate();
}

void WorldRenderManager::onUpdate()
{
	auto cx = round(m_camera->getPosition().x / WORLD_CHUNK_SIZE) - m_chunk_width / 2;
	auto cy = round(m_camera->getPosition().y / WORLD_CHUNK_SIZE) - m_chunk_height / 2;

	if (cx != last_cx || cy != last_cy)
	{
		last_cx = cx;
		last_cy = cy;

		std::set<int> toRemoveList;
		std::set<int> toLoadList;

		for (auto& iterator = m_offset_map.begin(); iterator != m_offset_map.end(); ++iterator) {
			toRemoveList.insert(iterator->first);//get all loaded chunks
		}
		for (int x = 0; x < m_chunk_width; x++) {
			for (int y = 0; y < m_chunk_height; y++)
			{
				auto targetX = cx + x;
				auto targetY = cy + y;
				if (
					targetX<0
					|| targetY<0
					|| targetX>=(m_world->getInfo().chunk_width)
					|| targetY>=(m_world->getInfo().chunk_height))
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
		for (auto& iterator = m_offset_map.begin(); iterator != m_offset_map.end(); ++iterator) {
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

void WorldRenderManager::render()
{
	if (Stats::light_enable)
		computeLight();

	auto& program = *ChunkMesh::getProgram();
	program.bind();
	ChunkMesh::getAtlas()->bind(0);
	ChunkMesh::getCornerAtlas()->bind(1);

	for (ChunkMeshInstance* mesh : m_chunks)
	{
		if (!(mesh->m_enabled))
			continue;

		auto worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(
			mesh->getPos().x - m_camera->getPosition().x,
			mesh->getPos().y - m_camera->getPosition().y, 0.0f));

		program.setUniformMat4("u_transform", getWorldViewMatrix()*worldMatrix);

		mesh->getVAO().bind();
		Call(glDrawArrays(GL_POINTS, 0, WORLD_CHUNK_AREA));

	}

	//light stuff
	if (!Stats::light_enable)
		return;

	renderLightMap();
	renderMainLightMap();

}

void WorldRenderManager::renderLightMap()
{
	m_light_frame_buffer->bind();
	{
		Call(glViewport(0, 0, m_chunk_width*WORLD_CHUNK_SIZE * 2, m_chunk_height*WORLD_CHUNK_SIZE * 2));
		Call(glClearColor(0.5f, 0.5f, 0.5f, 1));
		Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		m_light_simple_program->bind();
		m_light_simple_texture->bind(0);
		m_light_simple_VAO->bind();
		Call(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
		Call(glViewport(0, 0, Game::get().getWindow()->getWidth(), Game::get().getWindow()->getHeight()));
	}
	m_light_frame_buffer->unbind();
}

void WorldRenderManager::renderMainLightMap()
{

	auto worldMatrix = glm::mat4(1.0f);
	worldMatrix = glm::translate(worldMatrix, glm::vec3(
		last_cx*WORLD_CHUNK_SIZE - m_camera->getPosition().x,
		last_cy*WORLD_CHUNK_SIZE - m_camera->getPosition().y, 0.0f));
	worldMatrix = glm::scale(worldMatrix, glm::vec3(m_chunk_width*WORLD_CHUNK_SIZE, m_chunk_height*WORLD_CHUNK_SIZE, 1));


	m_light_program->bind();
	m_light_program->setUniformMat4("u_transform", getWorldViewMatrix() * worldMatrix);
	m_light_texture->bind(0);
	m_light_VAO->bind();

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	Call(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
	glDisable(GL_BLEND);
}


struct Pos
{
	int x, y;
};

void WorldRenderManager::computeLight()
{
	const half minLevel = 0.05f;
	/*static int ignore = 10;
	--ignore;
	if (ignore != 0)
		return;
	ignore = 100;*/

	clearLightMap();

	int blockX = last_cx * WORLD_CHUNK_SIZE;
	int blockY = last_cy * WORLD_CHUNK_SIZE;

	int test_light_x = m_camera->getPosition().x - blockX;
	//int test_light_x = 20;
	int test_light_y = m_camera->getPosition().y - blockY;
	//int test_light_y = 20;
	half lightPower = 2;


	//todo optimize vectors

	NDUtil::FifoList<Pos> list(500);
	NDUtil::FifoList<Pos> newList(500);
	auto current_list = &list;
	auto new_list = &newList;

	int blockOffsetX = last_cx * WORLD_CHUNK_SIZE;
	int blockOffsetY = last_cy * WORLD_CHUNK_SIZE;
	int maxX = m_chunk_width * WORLD_CHUNK_SIZE;
	int maxY = m_chunk_height * WORLD_CHUNK_SIZE;

	lightValue(test_light_x, test_light_y) = lightPower;
	current_list->push({ test_light_x, test_light_y });


	int runs = 0;
	while (!current_list->empty()) {
		current_list->popMode();
		while (!current_list->empty())
		{
			runs++;
			auto& p = current_list->pop();
			int x = p.x;
			int y = p.y;


			half l = lightValue(x, y) - getBlockOpacity(x + blockOffsetX, y + blockOffsetY);
			if (l < minLevel)
				continue;
			half newLightPower = l;




			//left
			int xm1 = x - 1;
			if (xm1 > 0)
			{
				half& v = lightValue(xm1, y);
				if (v < newLightPower) {
					v = newLightPower;
					new_list->push({ xm1, y });
				}
			}
			//down
			int ym1 = y - 1;
			if (ym1 > 0)
			{
				half& v = lightValue(x, ym1);
				if (v < newLightPower) {
					v = newLightPower;
					new_list->push({ x, ym1 });
				}
			}
			//right
			int x1 = x + 1;
			if (x1 < maxX)
			{
				half& v = lightValue(x1, y);
				if (v < newLightPower) {
					v = newLightPower;
					new_list->push({ x1, y });
				}
			}
			//up
			int y1 = y + 1;
			if (y1 < maxY)
			{
				half& v = lightValue(x, y1);
				if (v < newLightPower) {
					v = newLightPower;
					new_list->push({ x, y1 });
				}
			}
		}
		/*int i = m_chunk_width * m_chunk_height*WORLD_CHUNK_AREA * 4;
		while(i--)
		{
			m_light_map[i] /= lightPower;
		}*/

		auto t = current_list;
		t->clear();
		current_list = new_list;
		new_list = t;
	}

	m_light_simple_texture->setPixels(m_light_map);

	bool b = true;
	int g = 0;
	if (b)
		return;

	for (int y = 0; y < 100; y++)
	{
		for (int x = 0; x < 100; x++)
		{
			int b = (int)lightValue(x, y);
			std::cout << ((b > 9) ? "" : " ") << b;
		}
		std::cout << std::endl;
	}


}
