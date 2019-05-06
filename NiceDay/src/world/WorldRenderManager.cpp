#include "ndpch.h"
#include "WorldRenderManager.h"
#include "Game.h"
#include "graphics/Renderer.h"
#include "world/ChunkMeshInstance.h"
#include "glm/gtx/io.hpp"




WorldRenderManager::WorldRenderManager(Camera* cam, World* world)
	: m_world(world), m_camera(cam)
{
	onScreenResize();
}

WorldRenderManager::~WorldRenderManager()
{
	for (ChunkMeshInstance* m : m_chunks)
		delete m;
};

void WorldRenderManager::onScreenResize()
{
	int screenWidth = Game::get().getWindow()->getWidth();
	int screenHeight = Game::get().getWindow()->getHeight();

	float chunkwidth = (float)screenWidth / BLOCK_PIXEL_SIZE / WORLD_CHUNK_SIZE;
	float chunkheight = (float)screenHeight / BLOCK_PIXEL_SIZE / WORLD_CHUNK_SIZE;

	m_chunk_width = ceil(chunkwidth)+3;
	m_chunk_height = ceil(chunkheight)+3;

	m_chunks.reserve(getChunksSize());

	m_offset_map.clear();

	for (ChunkMeshInstance* m : m_chunks)
		m->m_enabled = false;

	while (m_chunks.size() < getChunksSize())
		m_chunks.push_back(new ChunkMeshInstance());


}

void WorldRenderManager::onUpdate()
{
	auto cx = World::getChunkCoord(m_camera->getPosition().x)- m_chunk_width/2;
	auto cy = World::getChunkCoord(m_camera->getPosition().y)- m_chunk_height/2;

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
				if (targetX<0
					|| targetY<0
					|| targetX>(m_world->getInfo().chunk_width - 1)
					|| targetY>(m_world->getInfo().chunk_height - 1))
					continue;
				StructChunkID mid = { Chunk::getChunkIDFromChunkPos(targetX, targetY) };
				int index = m_world->getChunkIndex(targetX, targetY);
				if(index==-1)
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
	auto& program = *ChunkMesh::getProgram();
	program.bind();
	ChunkMesh::getAtlas()->bind(0);
	ChunkMesh::getCornerAtlas()->bind(1);

	int screenWidth = Game::get().getWindow()->getWidth();
	int screenHeight = Game::get().getWindow()->getHeight();

	float ratio = (float)screenHeight / (float)screenWidth;

	float blockHeight = (float)screenHeight / BLOCK_PIXEL_SIZE;
	float chunkheight = (float)blockHeight / WORLD_CHUNK_SIZE;

	chunkheight = 1 / chunkheight;


	chunkheight /= WORLD_CHUNK_SIZE;

	//glm::mat4 trans(1.0f);



	for (ChunkMeshInstance* mesh : m_chunks)
	{
		if (!(mesh->m_enabled))
			continue;
		auto matrix = glm::scale(glm::mat4(1.0f), glm::vec3(ratio*chunkheight, chunkheight, 1));
		matrix = glm::translate(matrix, glm::vec3(
			mesh->getPos().x - m_camera->getPosition().x,
			mesh->getPos().y - m_camera->getPosition().y, 0.0f));
		program.setUniformMat4("u_transform", matrix);

		mesh->getVAO().bind();
		Call(glDrawArrays(GL_POINTS, 0, CHUNK_MESH_WIDTH * CHUNK_MESH_WIDTH));
	}
}
