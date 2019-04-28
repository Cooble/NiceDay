#include "ndpch.h"
#include "ChunkLoader.h"
#include "world/WorldIO.h"



ChunkLoader::ChunkLoader(World * w)
	:m_world(w)
{
}

ChunkLoader::~ChunkLoader()
{
}

static void printMap(std::unordered_map<int, int>& map) {
	for (auto& it = map.begin(); it != map.end(); it++) {
		int x, y;
		Chunk::getChunkPosFromID(it->first, x, y);
		ND_INFO("{}, {}",x,y);
	}
}
static void printSet(std::set<int>& set) {
	for (auto& it = set.begin(); it != set.end(); it++) {
		int x, y;
		Chunk::getChunkPosFromID(*it, x, y);
		ND_INFO("{}, {}", x, y);
	}
}

void ChunkLoader::tickInner()
{
	std::set<int> toLoadList;
	std::set<int> toRemoveList;
	

	auto& map = m_world->getMap();
	for (auto& iterator = map.begin(); iterator != map.end(); iterator++) {
			toRemoveList.insert(iterator->first);//get all loaded chunks
	}

	for (EntityWrapper& w : m_loader_entities) {
		auto pos = w.e->getPosition();

		int newID = Chunk::getChunkIDFromWorldPos((int)pos.x, (int)pos.y);
		int newRadius = w.e->getChunkRadius();

		w.last_chunk_id = newID;
		w.last_chunk_radius = newRadius;

		int cx = (int)pos.x >> WORLD_CHUNK_BIT_SIZE;
		int cy = (int)pos.y >> WORLD_CHUNK_BIT_SIZE;

		for (int x = cx - newRadius; x < cx + newRadius; x++) {
			if ((x < 0) || (x > m_world->getInfo().chunk_width - 1))
				continue;
			for (int y = cy - newRadius; y < cy + newRadius; y++) {
				if ((y < 0) || (y > m_world->getInfo().chunk_height - 1))
					continue;
				if (!m_world->isChunkLoaded(x, y))
					toLoadList.insert(Chunk::getChunkIDFromChunkPos(x, y));

				toRemoveList.erase(Chunk::getChunkIDFromChunkPos(x, y));
			}
		}
	}
	m_world->unloadChunks(toRemoveList);
	m_world->loadChunks(toLoadList);
}

void ChunkLoader::onUpdate()
{
	for (const EntityWrapper& w : m_loader_entities) {
		auto pos = w.e->getPosition();
		int newID = Chunk::getChunkIDFromWorldPos((int)pos.x, (int)pos.y);

		int cx = (int)pos.x >> WORLD_CHUNK_BIT_SIZE;
		int cy = (int)pos.y >> WORLD_CHUNK_BIT_SIZE;

		int newRadius = w.e->getChunkRadius();
		if (newID != w.last_chunk_id || newRadius != w.last_chunk_radius) {//we have change here!
			tickInner();
			return;
		}
	}
}

void ChunkLoader::registerEntity(IChunkLoaderEntity * e)
{
	EntityWrapper w = { e, -1, 0 };
	m_loader_entities.push_back(w);//does it work?
}

void ChunkLoader::unregisterEntity(IChunkLoaderEntity * e)
{
	int i = 0;
	for (const EntityWrapper& w : m_loader_entities) {
		if (w.e == e) {//todo does it work?
			m_loader_entities.erase(m_loader_entities.begin() + i);
			return;
		}
		i++;
	}
}

