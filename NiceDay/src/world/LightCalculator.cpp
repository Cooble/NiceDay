#include "ndpch.h"
#include "LightCalculator.h"
#include "World.h"

template<typename T>
void clear(std::queue<T> &q)
{
	std::queue<T> empty;
	std::swap(q, empty);
}

struct Pos
{
	int x, y;
};

LightCalculator::LightCalculator(World* world)
	:m_running(false), m_world(world),m_map(nullptr),m_done_map(nullptr)
{
}

LightCalculator::~LightCalculator()
{
	delete[] m_map;
	delete[] m_done_map;
}



void LightCalculator::registerLight(LightSource* light){m_sources.push_back(light);}
void LightCalculator::removeLight(LightSource* light)
{
	for (int i = 0; i < m_sources.size(); i++)
	{
		if (m_sources[i] == light)
		{
			m_sources.erase(m_sources.begin() + i);
			return;
		}
	}
}
void LightCalculator::setChunkOffset(int x, int y)
{
	m_chunk_offset_x = x;
	m_chunk_offset_y = y;
}
void LightCalculator::setDimensions(int chunkWidth, int chunkHeight)
{
	m_chunk_width = chunkWidth;
	m_chunk_height = chunkHeight;
}
void LightCalculator::snapshot()
{
	std::lock_guard<std::mutex> guard(m_snapshot_queue_mutex);

	//clear buffer to max size
	while (m_snapshot_queue.size() > MAX_SNAPSHOT_NUMBER - 1)
	{
		auto poin = m_snapshot_queue.front();
		m_snapshot_queue.pop();
		delete poin;
	}

	auto sn = new Snapshot();
	auto& snap = sn->data;
	for (auto light : m_sources)
	{
		auto pos = light->getLightPosition();

		snap.push_back({ light->getIntensity(), pos.first, pos.second });
	}
	sn->offsetX = m_chunk_offset_x;
	sn->offsetY = m_chunk_offset_y;
	sn->chunkWidth = m_chunk_width;
	sn->chunkHeight = m_chunk_height;

	m_snapshot_queue.push(sn);

}



//threading ==========================================================================
void LightCalculator::run()
{
	std::thread t(&LightCalculator::runInner, this);
	t.detach();//fly little birdie daemon, fly
}

void LightCalculator::runInner()
{
	m_running = true;
	while (m_running) {
		if (m_chunk_width != m_snap_width || m_chunk_height != m_snap_height)
		{
			std::unique_lock<std::mutex> guard(m_snapshot_queue_mutex);
			clear(m_snapshot_queue);//change in dimensions means throw away all previous snapshots
			guard.unlock();
			m_snap_width = m_chunk_width;
			m_snap_height = m_chunk_height;
			setDimensionsInner();//create new maps
		}

		if (!m_snapshot_queue.empty())
		{
			//make copy of snapshot
			//nobody will modify queue when I'm copying
			std::unique_lock<std::mutex> guard(m_snapshot_queue_mutex);
			auto originalSnap = m_snapshot_queue.front();
			auto snap = Snapshot(*originalSnap);
			m_snapshot_queue.pop();
			delete originalSnap;
			guard.unlock();

			computeLight(snap);

			//swap buffers
			auto t = m_done_map;
			m_done_map = m_map;
			m_map = t;
			m_done_ch_offset = std::make_pair(snap.offsetX, snap.offsetY);
			m_is_fresh_map = true;//notify that new map was rendered
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));//take a nap
	}
}

void LightCalculator::stop() { m_running = false; }

half& LightCalculator::lightValue(int x, int y)
{
	return m_map[y * m_snap_width * WORLD_CHUNK_SIZE + x];
}

float LightCalculator::getBlockOpacity(int x, int y)
{
	auto b = m_world->getLoadedBlockPointer(x, y);
	if (b)
		return BlockRegistry::get().getBlock(b->block_id).getOpacity(*b);
	return 1000; //outside map -> no light
}

void LightCalculator::computeLight(Snapshot& sn)
{
	const half minLevel = 0.05f;
	const int defaultListSize = 500;
	memset(m_map, 0, sn.chunkWidth*sn.chunkHeight*WORLD_CHUNK_AREA * sizeof(half));

	int blockOffsetX = sn.offsetX * WORLD_CHUNK_SIZE;
	int blockOffsetY = sn.offsetY * WORLD_CHUNK_SIZE;

	NDUtil::FifoList<Pos> list(defaultListSize);
	NDUtil::FifoList<Pos> newList(defaultListSize);
	auto current_list = &list;
	auto new_list = &newList;

	int maxX = m_chunk_width * WORLD_CHUNK_SIZE;
	int maxY = m_chunk_height * WORLD_CHUNK_SIZE;

	//add all light sources to map
	for (auto& light : sn.data)
	{
		lightValue(light.x - blockOffsetX, light.y - blockOffsetY) = light.intensity;
		current_list->push({ light.x - blockOffsetX, light.y - blockOffsetY });
	}

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
		auto t = current_list;
		t->clear();
		current_list = new_list;
		new_list = t;
	}
}


void LightCalculator::setDimensionsInner()
{
	if (m_map)
		delete[] m_map;
	if (m_done_map)
		delete[] m_done_map;

	m_map = new half[m_snap_width*m_snap_height*WORLD_CHUNK_AREA];
	m_done_map = new half[m_snap_width*m_snap_height*WORLD_CHUNK_AREA];
}
