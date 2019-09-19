#include "ndpch.h"
#include "ChunkLoader.h"



ChunkLoader::ChunkLoader(World * w)
	:m_world(w)
{
}

ChunkLoader::~ChunkLoader()= default;

/*static void printMap(std::unordered_map<int, int>& map) {
	for (auto it = map.begin(); it != map.end(); ++it) {
		int x, y;
		Chunk::getChunkPosFromID(it->first, x, y);
		ND_INFO("{}, {}",x,y);
	}
}
static void printSet(std::set<int>& set) {
	for (auto it = set.begin(); it != set.end(); ++it) {
		int x, y;
		Chunk::getChunkPosFromID(*it, x, y);
		ND_INFO("{}, {}", x, y);
	}
}*/

void ChunkLoader::tickInner()
{
	std::set<int> toLoadList;
	std::set<int> toRemoveList;
	

	auto& map = m_world->getMap();
	for (auto& iterator : map)
	{
		half_int c = iterator.first;
		auto& g = *m_world->getChunk(c.x, c.y);
		if(!g.isLocked())//cannot unload chunk that is locked
			toRemoveList.insert(iterator.first);//get all loaded chunks
	}
	

	for (EntityWrapper& w : m_loader_entities) {
		auto pos = w.e->getPosition();

		int newID = Chunk::getChunkIDFromWorldPos((int)pos.x, (int)pos.y);
		half_int newRadius = w.e->getChunkRadius();

		w.last_chunk_id = newID;
		w.last_chunk_radius = newRadius;

		int cx = (int)pos.x >> WORLD_CHUNK_BIT_SIZE;
		int cy = (int)pos.y >> WORLD_CHUNK_BIT_SIZE;

		for (int x = cx - newRadius.x; x < cx + newRadius.x; x++) {
			if ((x < 0) || (x >= m_world->getInfo().chunk_width))
				continue;
			for (int y = cy - newRadius.y; y < cy + newRadius.y; y++) {
				if ((y < 0) || (y >= m_world->getInfo().chunk_height))
					continue;

				if (!m_world->isChunkFullyLoaded(half_int(x,y)))
					toLoadList.insert(half_int(x, y));

				toRemoveList.erase(half_int(x, y));
			}
		}
	}
	if (!toRemoveList.empty())
		m_world->unloadChunks(toRemoveList);
	if(!toLoadList.empty())
		m_world->loadChunksAndGen(toLoadList);
}

void ChunkLoader::onUpdate()
{
	if (m_dirty) {
		m_dirty = false;
		tickInner();
		return;
	}
	for (const EntityWrapper& w : m_loader_entities) {
		auto pos = w.e->getPosition();
		int newID = Chunk::getChunkIDFromWorldPos((int)pos.x, (int)pos.y);

		int cx = (int)pos.x >> WORLD_CHUNK_BIT_SIZE;
		int cy = (int)pos.y >> WORLD_CHUNK_BIT_SIZE;

		half_int newRadius = w.e->getChunkRadius();
		if (newID != w.last_chunk_id || newRadius != w.last_chunk_radius) {//we have change here!
			tickInner();
			return;
		}
	}
}

void ChunkLoader::registerEntity(IChunkLoaderEntity * e)
{
	m_dirty = true;
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

void ChunkLoader::clearEntities()
{
	m_loader_entities.clear();
	tickInner();
}

