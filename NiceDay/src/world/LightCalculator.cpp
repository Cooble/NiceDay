#include "ndpch.h"
#include "LightCalculator.h"
#include "World.h"
#include "Stats.h"
#include "biome/Biome.h"
#include "biome/BiomeRegistry.h"

constexpr int DEFAULT_POS_LIST_SIZE = 500;

template <typename T>
void clear(std::queue<T>& q)
{
	std::queue<T> empty;
	std::swap(q, empty);
}

LightCalculator::LightCalculator(World* world)
	: m_light_list0(DEFAULT_POS_LIST_SIZE),
	  m_light_list1(DEFAULT_POS_LIST_SIZE),
	  m_light_list0_main_thread(DEFAULT_POS_LIST_SIZE),
	  m_light_list1_main_thread(DEFAULT_POS_LIST_SIZE),
	  m_running(false),
	  m_world(world),
	  m_map(nullptr),
	  m_done_map(nullptr)
{
}

LightCalculator::~LightCalculator()
{
	delete[] m_map;
	delete[] m_done_map;
}


void LightCalculator::registerLight(LightSource* light) { m_sources.push_back(light); }

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

		snap.push_back({light->getIntensity(), pos.first, pos.second});
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
	t.detach(); //fly little birdie daemon, fly
}

void LightCalculator::runInner()
{
	m_running = true;
	while (m_running)
	{
		if (m_chunk_width != m_snap_width || m_chunk_height != m_snap_height)
		{
			std::unique_lock<std::mutex> guard(m_snapshot_queue_mutex);
			clear(m_snapshot_queue); //change in dimensions means throw away all previous snapshots
			guard.unlock();
			m_snap_width = m_chunk_width;
			m_snap_height = m_chunk_height;
			setDimensionsInner(); //create new maps
		}

		if (!m_snapshot_queue.empty())
		{
			using namespace std::chrono;

			//make copy of snapshot
			//nobody will modify queue when I'm copying
			std::unique_lock<std::mutex> guard(m_snapshot_queue_mutex);
			auto originalSnap = m_snapshot_queue.front();
			auto snap = Snapshot(*originalSnap);
			m_snapshot_queue.pop();
			delete originalSnap;
			guard.unlock();

			auto last = system_clock::now();
			computeLight(snap);

			//swap buffers
			auto t = m_done_map;
			m_done_map = m_map;
			m_map = t;
			m_done_ch_offset = std::make_pair(snap.offsetX, snap.offsetY);
			m_is_fresh_map = true; //notify that new map was rendered
			Stats::light_millis = duration_cast<milliseconds>(system_clock::now() - last).count();
		}
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));//take a nap. never:D
	}
}

void LightCalculator::stop() { m_running = false; }

half& LightCalculator::lightValue(int x, int y)
{
	return m_map[y * m_snap_width * WORLD_CHUNK_SIZE + x];
}

half LightCalculator::getBlockOpacity(int x, int y)
{
	auto b = m_world->getLoadedBlockPointer(x, y);
	if (b)
		return BlockRegistry::get().getBlock(b->block_id).getOpacity();
	return 1000; //outside map -> no light
}

uint8_t& LightCalculator::blockLightLevel(int x, int y)
{
	static uint8_t lasagne = 255;
	//todo this lasagna is causing problems because it is seen as light src on boundary ...we know
	//just to prevent problems with not loaded chunks/return so high light that no other updates will be neccessary
	auto b = m_world->getLoadedChunkPointerNoConst(x >> WORLD_CHUNK_BIT_SIZE, y >> WORLD_CHUNK_BIT_SIZE);
	if (b)
		return b->getLightLevel(x & (WORLD_CHUNK_SIZE - 1), y & (WORLD_CHUNK_SIZE - 1));
	return lasagne; //outside map -> no light
}
uint8_t& LightCalculator::blockLightLevelDefault0(int x, int y)
{
	static uint8_t lasagne = 0;//if not loaded return 0
	//todo this lasagna is causing problems because it is seen as light src on boundary ...we know
	//just to prevent problems with not loaded chunks/return so high light that no other updates will be neccessary
	auto b = m_world->getLoadedChunkPointerNoConst(x >> WORLD_CHUNK_BIT_SIZE, y >> WORLD_CHUNK_BIT_SIZE);
	if (b)
		return b->getLightLevel(x & (WORLD_CHUNK_SIZE - 1), y & (WORLD_CHUNK_SIZE - 1));
	return lasagne; //outside map -> no light
}

void LightCalculator::computeLightOld(Snapshot& sn)
{
	const half minLevel = 1;
	const int defaultListSize = 500;
	memset(m_map, 0, sn.chunkWidth * sn.chunkHeight * WORLD_CHUNK_AREA * sizeof(half));

	int blockOffsetX = sn.offsetX * WORLD_CHUNK_SIZE;
	int blockOffsetY = sn.offsetY * WORLD_CHUNK_SIZE;

	m_light_list0.clear();
	m_light_list1.clear();

	auto current_list = &m_light_list0;
	auto new_list = &m_light_list1;

	int maxX = m_chunk_width * WORLD_CHUNK_SIZE;
	int maxY = m_chunk_height * WORLD_CHUNK_SIZE;

	//add all light sources to map
	for (auto& light : sn.data)
	{
		lightValue(light.x - blockOffsetX, light.y - blockOffsetY) = light.intensity;
		current_list->push({light.x - blockOffsetX, light.y - blockOffsetY});
	}

	int runs = 0;
	while (!current_list->empty())
	{
		current_list->popMode();
		while (!current_list->empty())
		{
			runs++;
			auto& p = current_list->pop();
			const int& x = p.x;
			const int& y = p.y;

			auto val = lightValue(x, y);
			auto opacity = getBlockOpacity(x + blockOffsetX, y + blockOffsetY);
			half l = val - opacity;

			if (val < opacity) //we had overflow -> dark is coming for us all
				continue;

			half newLightPower = l;

			//left
			int xm1 = x - 1;
			if (xm1 > 0)
			{
				half& v = lightValue(xm1, y);
				if (v < newLightPower)
				{
					v = newLightPower;
					new_list->push({xm1, y});
				}
			}
			//down
			int ym1 = y - 1;
			if (ym1 > 0)
			{
				half& v = lightValue(x, ym1);
				if (v < newLightPower)
				{
					v = newLightPower;
					new_list->push({x, ym1});
				}
			}
			//right
			int x1 = x + 1;
			if (x1 < maxX)
			{
				half& v = lightValue(x1, y);
				if (v < newLightPower)
				{
					v = newLightPower;
					new_list->push({x1, y});
				}
			}
			//up
			int y1 = y + 1;
			if (y1 < maxY)
			{
				half& v = lightValue(x, y1);
				if (v < newLightPower)
				{
					v = newLightPower;
					new_list->push({x, y1});
				}
			}
		}
		auto t = current_list;
		t->clear();
		current_list = new_list;
		new_list = t;
	}
	updateMap(sn);
}

void LightCalculator::computeChunk(Chunk& c) //will be called on chunkgeneration
{
	auto current_list = &m_light_list0_main_thread;
	auto new_list = &m_light_list1_main_thread;

	current_list->clear();
	new_list->clear();
	//todo maybe lock chunk to prevent main thread from unloading it while working

	uint8_t backLight = BiomeRegistry::get().getBiome(c.getBiome()).getBackgroundLight();
	//set whole chunk to dark and find lightsources
	if (backLight == 0)
	{
		for (int x = 0; x < WORLD_CHUNK_SIZE; ++x)
		{
			for (int y = 0; y < WORLD_CHUNK_SIZE; ++y)
			{
				BlockStruct& b = c.getBlock(x, y);
				uint8_t val = BlockRegistry::get().getBlock(b.block_id).getLightSrcVal();
				c.getLightLevel(x, y) = val;
				if (val == 0)
					continue;
				current_list->push({x, y});
			}
		}
	}
	else
	{
		for (int x = 0; x < WORLD_CHUNK_SIZE; ++x)
		{
			for (int y = 0; y < WORLD_CHUNK_SIZE; ++y)
			{
				auto& b = c.getBlock(x, y);
				auto& bl = BlockRegistry::get().getBlock(b.block_id);
				uint8_t val = bl.getLightSrcVal();
				//check if ambient light will be there
				if (bl.getOpacity() <= 1 && val < backLight && (b.isWallFree()||BlockRegistry::get().getWall(b.wallID()).isTransparent()))
				{
					c.getLightLevel(x, y) = backLight;
					current_list->push({x, y});
				}
				else
				{
					c.getLightLevel(x, y) = val;
					if (val == 0)
						continue;
					current_list->push({x, y});
				}
			}
		}
	}
	int runs = 0;

	//lets flood the chunk with eternal light
	while (!current_list->empty())
	{
		current_list->popMode();
		while (!current_list->empty())
		{
			runs++;
			auto& p = current_list->pop();
			const int& x = p.x;
			const int& y = p.y;

			auto val = c.getLightLevel(x, y);

			auto opacity = BlockRegistry::get().getBlock(c.getBlock(x, y).block_id).getOpacity();
			half newLightPower = val - opacity;

			if (val < opacity) //we had overflow -> dark is coming for us all
				continue;

			//left
			int xm1 = x - 1;
			if (xm1 >= 0)
			{
				half& v = c.getLightLevel(xm1, y);
				if (v < newLightPower)
				{
					v = newLightPower;
					new_list->push({xm1, y});
				}
			}
			//down
			int ym1 = y - 1;
			if (ym1 >= 0)
			{
				half& v = c.getLightLevel(x, ym1);
				if (v < newLightPower)
				{
					v = newLightPower;
					new_list->push({x, ym1});
				}
			}
			//right
			int x1 = x + 1;
			if (x1 < WORLD_CHUNK_SIZE)
			{
				half& v = c.getLightLevel(x1, y);
				if (v < newLightPower)
				{
					v = newLightPower;
					new_list->push({x1, y});
				}
			}
			//up
			int y1 = y + 1;
			if (y1 < WORLD_CHUNK_SIZE)
			{
				half& v = c.getLightLevel(x, y1);
				if (v < newLightPower)
				{
					v = newLightPower;
					new_list->push({x, y1});
				}
			}
		}
		auto t = current_list;
		t->clear();
		current_list = new_list;
		new_list = t;
	}
}

void LightCalculator::computeChunkBorders(Chunk& c)
{
	auto current_list = &m_light_list0_main_thread;
	auto new_list = &m_light_list1_main_thread;

	current_list->clear();
	new_list->clear();

	int cx = half_int::getX(c.chunkID());
	int cy = half_int::getY(c.chunkID());
	int minX = cx * WORLD_CHUNK_SIZE;
	int minY = cy * WORLD_CHUNK_SIZE;

	Chunk* up = m_world->getLoadedChunkPointerNoConst(cx, cy + 1);
	Chunk* down = m_world->getLoadedChunkPointerNoConst(cx, cy - 1);
	Chunk* left = m_world->getLoadedChunkPointerNoConst(cx - 1, cy);
	Chunk* right = m_world->getLoadedChunkPointerNoConst(cx + 1, cy);
	//add all external boundary light blocks
	if (up)
		for (int i = 0; i < WORLD_CHUNK_SIZE; ++i)
			current_list->push({minX + i, minY + WORLD_CHUNK_SIZE + 0});
	if (down)
		for (int i = 0; i < WORLD_CHUNK_SIZE; ++i)
			current_list->push({minX + i, minY - 1});
	if (left)
		for (int i = 0; i < WORLD_CHUNK_SIZE; ++i)
			current_list->push({minX - 1, minY + i});
	if (right)
		for (int i = 0; i < WORLD_CHUNK_SIZE; ++i)
			current_list->push({minX + WORLD_CHUNK_SIZE, minY + i});

	int runs = 0;
	while (!current_list->empty())
	{
		current_list->popMode();
		while (!current_list->empty())
		{
			runs++;
			auto& p = current_list->pop();
			const int& x = p.x;
			const int& y = p.y;

			auto val = blockLightLevel(x, y);
			auto opacity = getBlockOpacity(x, y);
			half l = val - opacity;

			if (val < opacity) //we had overflow -> dark is coming for us all
				continue;

			half newLightPower = l;

			//left
			int xm1 = x - 1;

			half* v = &blockLightLevel(xm1, y);
			if (*v < newLightPower)
			{
				*v = newLightPower;
				new_list->push({xm1, y});
			}

			//down
			int ym1 = y - 1;
			v = &blockLightLevel(x, ym1);
			if (*v < newLightPower)
			{
				*v = newLightPower;
				new_list->push({x, ym1});
			}
			//right
			int x1 = x + 1;
			v = &blockLightLevel(x1, y);
			if (*v < newLightPower)
			{
				*v = newLightPower;
				new_list->push({x1, y});
			}
			//up
			int y1 = y + 1;
			v = &blockLightLevel(x, y1);
			if (*v < newLightPower)
			{
				*v = newLightPower;
				new_list->push({x, y1});
			}
		}
		auto t = current_list;
		t->clear();
		current_list = new_list;
		new_list = t;
	}
}

void LightCalculator::computeChange(int xx, int yy)
{
	constexpr int maxLightRadius = 17; //true refresh area is one less
	auto current_list = &m_light_list0_main_thread;
	auto new_list = &m_light_list1_main_thread;

	current_list->clear();
	new_list->clear();
	//todo maybe lock chunk to prevent main thread from unloading it while working
	xx -= maxLightRadius;
	//set whole chunk to dark and find lightsources
	for (int x = 0; x < maxLightRadius * 2; ++x)
	{
		int worldX = x + xx;
		int yrad = x < maxLightRadius ? x : 2 * maxLightRadius - x;
		for (int y = -yrad; y < yrad + 1; ++y)
		{
			int worldY = yy + y;
			if (y == -yrad || y == yrad)
			{
				//todo maybe check if lightsrcval == 0
				current_list->push({worldX, worldY}); //add external boundary light
				continue;
			}

			Chunk* c = m_world->getLoadedChunkPointerNoConst(worldX >> WORLD_CHUNK_BIT_SIZE,
			                                                 worldY >> WORLD_CHUNK_BIT_SIZE);
			if (c==nullptr)
				continue;

			uint8_t backLight = BiomeRegistry::get(). getBiome(c->getBiome()).getBackgroundLight();

			int chunkX = worldX & (BIT(WORLD_CHUNK_BIT_SIZE) - 1);
			int chunkY = worldY & (BIT(WORLD_CHUNK_BIT_SIZE) - 1);

			auto& b = c->getBlock(chunkX, chunkY);

			auto& bl = BlockRegistry::get().getBlock(b.block_id);
			uint8_t val = bl.getLightSrcVal();
			//check if ambient light will be there
			if (bl.getOpacity() <= 1 && val < backLight && (b.isWallFree() || BlockRegistry::get().getWall(b.wallID()).isTransparent()))
			{
				c->getLightLevel(chunkX, chunkY) = backLight;
				current_list->push({ worldX, worldY });
			}
			else
			{
				c->getLightLevel(chunkX, chunkY) = val;
				if (val == 0)
					continue;
				current_list->push({ worldX, worldY });
			}
		}
	}

	//flood the eternal light
	int runs = 0;
	while (!current_list->empty())
	{
		current_list->popMode();
		while (!current_list->empty())
		{
			runs++;
			auto& p = current_list->pop();
			const int& x = p.x;
			const int& y = p.y;

			auto val = blockLightLevelDefault0(x, y);
			auto opacity = getBlockOpacity(x, y);
			half l = val - opacity;

			if (val < opacity) //we had overflow -> dark is coming for us all
				continue;

			half newLightPower = l;

			//left
			int xm1 = x - 1;

			half* v = &blockLightLevel(xm1, y);
			if (*v < newLightPower)
			{
				*v = newLightPower;
				new_list->push({xm1, y});
			}

			//down
			int ym1 = y - 1;
			v = &blockLightLevel(x, ym1);
			if (*v < newLightPower)
			{
				*v = newLightPower;
				new_list->push({x, ym1});
			}
			//right
			int x1 = x + 1;
			v = &blockLightLevel(x1, y);
			if (*v < newLightPower)
			{
				*v = newLightPower;
				new_list->push({x1, y});
			}
			//up
			int y1 = y + 1;
			v = &blockLightLevel(x, y1);
			if (*v < newLightPower)
			{
				*v = newLightPower;
				new_list->push({x, y1});
			}
		}
		auto t = current_list;
		t->clear();
		current_list = new_list;
		new_list = t;
	}
}

void LightCalculator::computeLight(Snapshot& sn)
{
	updateMap(sn);//set map content to cached chunk light

	//add dynamic lighting
	m_light_list0.clear();
	m_light_list1.clear();

	auto current_list = &m_light_list0;
	auto new_list = &m_light_list1;

	int minX = sn.offsetX * WORLD_CHUNK_SIZE;
	int minY = sn.offsetY * WORLD_CHUNK_SIZE;

	int maxX = minX + m_chunk_width * WORLD_CHUNK_SIZE;
	int maxY = minY + m_chunk_height * WORLD_CHUNK_SIZE;

	int width = m_chunk_width * WORLD_CHUNK_SIZE;
	int height = m_chunk_height* WORLD_CHUNK_SIZE;

	//add dynamic light sources to map
	for (auto& light : sn.data)
	{
		if (light.x >= minX && light.x < maxX && light.y >= minY && light.y < maxY)
		{
			auto& l = lightValue(light.x- minX, light.y-minY);
			if(l<light.intensity)
			{
				l = light.intensity;
				current_list->push({light.x - minX, light.y - minY });
			}
		}
	}

	int runs = 0;
	while (!current_list->empty())
	{
		current_list->popMode();
		while (!current_list->empty())
		{
			runs++;
			auto& p = current_list->pop();
			const int& x = p.x;
			const int& y = p.y;

			auto val = lightValue(x, y);
			auto opacity = getBlockOpacity(minX+x, minY+y);
			half newLightPower = val - opacity;

			if (val < opacity) //we had overflow -> dark is coming for us all
				continue;

			//left
			int xm1 = x - 1;
			if (xm1 >= 0)
			{
				half& v = lightValue(xm1, y);
				if (v < newLightPower)
				{
					v = newLightPower;
					new_list->push({xm1, y});
				}
			}
			//down
			int ym1 = y - 1;
			if (ym1 >= 0)
			{
				half& v = lightValue(x, ym1);
				if (v < newLightPower)
				{
					v = newLightPower;
					new_list->push({x, ym1});
				}
			}
			//right
			int x1 = x + 1;
			if (x1 < width)
			{
				half& v = lightValue(x1, y);
				if (v < newLightPower)
				{
					v = newLightPower;
					new_list->push({x1, y});
				}
			}
			//up
			int y1 = y + 1;
			if (y1 < height)
			{
				half& v = lightValue(x, y1);
				if (v < newLightPower)
				{
					v = newLightPower;
					new_list->push({x, y1});
				}
			}
		}
		auto t = current_list;
		t->clear();
		current_list = new_list;
		new_list = t;
	}
}

void LightCalculator::updateMap(Snapshot& sn)
{
	//todo this needs to use memcpy or death will feast upon us all
	for (int cx = 0; cx < sn.chunkWidth; ++cx)
	{
		for (int cy = 0; cy < sn.chunkHeight; ++cy)
		{
			auto c = m_world->getLoadedChunkPointer(cx + sn.offsetX, cy + sn.offsetY);
			if (c)
				for (int x = 0; x < WORLD_CHUNK_SIZE; ++x)
				{
					for (int y = 0; y < WORLD_CHUNK_SIZE; ++y)
					{
						lightValue(cx * WORLD_CHUNK_SIZE + x, cy * WORLD_CHUNK_SIZE + y) = c->getLightLevel(x, y);
					}
				}
			else
			{
				for (int x = 0; x < WORLD_CHUNK_SIZE; ++x)
				{
					for (int y = 0; y < WORLD_CHUNK_SIZE; ++y)
					{
						lightValue(cx * WORLD_CHUNK_SIZE + x, cy * WORLD_CHUNK_SIZE + y) = 0;
					}
				}
			}
		}
	}
}

void LightCalculator::darken(Snapshot& sn)//deprecated
{
	//todo this needs to use memcpy or death will feast upon us all
	for (int cx = 0; cx < sn.chunkWidth; ++cx)
	{
		for (int cy = 0; cy < sn.chunkHeight; ++cy)
		{
			auto c = m_world->getLoadedChunkPointerNoConst(cx + sn.offsetX, cy + sn.offsetY);
			if (c)
				for (int x = 0; x < WORLD_CHUNK_SIZE; ++x)
				{
					for (int y = 0; y < WORLD_CHUNK_SIZE; ++y)
					{
						c->getLightLevel(x, y) = 0;
						//	auto& matic = blockLightLevel((sn.offsetX + cx) * WORLD_CHUNK_SIZE + x, (sn.offsetY + cy) * WORLD_CHUNK_SIZE + y);
						//	matic = 0;
					}
				}
		}
	}
}

void LightCalculator::setDimensionsInner()
{
	if (m_map)
		delete[] m_map;
	if (m_done_map)
		delete[] m_done_map;

	m_map = new half[m_snap_width * m_snap_height * WORLD_CHUNK_AREA];
	m_done_map = new half[m_snap_width * m_snap_height * WORLD_CHUNK_AREA];
}
