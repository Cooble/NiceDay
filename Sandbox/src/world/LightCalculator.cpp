#include "ndpch.h"
#include "LightCalculator.h"
#include "World.h"
#include "core/Stats.h"
#include "biome/Biome.h"
#include "biome/BiomeRegistry.h"

using namespace nd;

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
	  m_light_list3(DEFAULT_POS_LIST_SIZE),
	  m_light_list2(DEFAULT_POS_LIST_SIZE),
	  m_light_list0_main_thread(DEFAULT_POS_LIST_SIZE),
	  m_light_list1_main_thread(DEFAULT_POS_LIST_SIZE),
	  m_world(world),
	  m_big_buffer(nullptr),
	  m_map(nullptr),
	  m_map_done(nullptr),
	  m_map_sky(nullptr),
	  m_map_sky_out_done(nullptr),
	  m_map_sky_out(nullptr)
{
}

LightCalculator::~LightCalculator()
{
	stop();
};

void LightCalculator::assignComputeChange(int x, int y)
{
	{
		std::lock_guard<std::mutex> l(m_cached_light_assign_mutex);
		Assignment p = {x, y, Assignment::CHANGE};
		m_cached_light_assignments.emplace(p);
	}
	m_wait_condition_variable.notify_one();
}

void LightCalculator::assignComputeChunk(int cx, int cy)
{
	{
		std::lock_guard<std::mutex> l(m_cached_light_assign_mutex);
		Assignment p = {cx, cy, Assignment::CHUNK};
		m_cached_light_assignments.emplace(p);
	}
	m_wait_condition_variable.notify_one();
}

void LightCalculator::assignComputeChunkBorders(int cx, int cy,const ChunkPack& res)
{
	{
		std::lock_guard<std::mutex> l(m_cached_light_assign_mutex);
		Assignment p = {cx, cy, Assignment::BORDERS,res};
		m_cached_light_assignments.emplace(p);
	}
	m_wait_condition_variable.notify_one();
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

void LightCalculator::ChunkQuadro::assignLightJob(BlockAccess& access)
{
	for (int i = 0; i < 5; ++i)
	{
		auto pair = src[i];
		auto c = access.getChunkM(pair.first, pair.second);
		if (c)
			c->getLightJob().assign();
		else //invalidate all unreachable chunks
			src[i] = std::make_pair(-1, -1);
	}
}
void LightCalculator::ChunkQuadro::markDoneLightJob(BlockAccess& access)
{
	for (auto pair : src)
	{
		auto c = access.getChunkM(pair.first, pair.second);
		if (c)
			c->getLightJob().markDone();
	}
}

void LightCalculator::setChunkOffset(int x, int y)
{
	m_chunk_offset_x = x;
	m_chunk_offset_y = y;
}

void LightCalculator::setDimensions(int chunkWidth, int chunkHeight)
{
	if (m_chunk_width != chunkWidth || m_chunk_height != chunkHeight)
	{
		m_chunk_width = chunkWidth;
		m_chunk_height = chunkHeight;
		m_wait_condition_variable.notify_one();
	}
}

void LightCalculator::snapshot()
{
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

			//snap.push_back({light->getIntensity(), pos.first, pos.second});
			snap.push_back({light->getIntensity(), pos.first, pos.second});
		}
		sn->offsetX = m_chunk_offset_x;
		sn->offsetY = m_chunk_offset_y;
		sn->chunkWidth = m_chunk_width;
		sn->chunkHeight = m_chunk_height;

		m_snapshot_queue.push(sn);
	}
	m_wait_condition_variable.notify_one(); //we have to notify light thread that changes were made
}


//threading ==========================================================================
void LightCalculator::run()
{
	m_t = std::thread(&LightCalculator::runInnerLT, this);
	//t.detach(); //fly little birdie daemon, fly
}

void LightCalculator::runInnerLT()
{
	std::mutex waitMutex;
	std::unique_lock<std::mutex> loopLock(waitMutex);
	m_running = true;
	while (m_running)
	{
		m_wait_condition_variable.wait(loopLock);
		if (m_chunk_width != m_snap_width || m_chunk_height != m_snap_height)
		{
			std::unique_lock<std::mutex> guard(m_snapshot_queue_mutex);
			clear(m_snapshot_queue); //change in dimensions means throw away all previous snapshots
			guard.unlock();
			m_snap_width = m_chunk_width;
			m_snap_height = m_chunk_height;
			setDimensionsInnerLT(); //create new maps
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
			computeLT(snap);

			//swap buffers
			auto t = m_map_done;
			m_map_done = m_map;
			m_map = t;

			//swap sky buffers
			t = m_map_sky_out_done;
			m_map_sky_out_done = m_map_sky_out;
			m_map_sky_out = t;

			m_done_ch_offset = std::make_pair(snap.offsetX, snap.offsetY);
			m_is_fresh_map = true; //notify that new map was rendered
			Stats::light_millis = duration_cast<milliseconds>(system_clock::now() - last).count();
		}
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));//take a nap. never:D
	}
	if (m_big_buffer) {
		delete[] m_big_buffer;
		m_big_buffer = nullptr;
	}
}

void LightCalculator::stop()
{
	if (m_running)
	{
		m_running = false;
		m_wait_condition_variable.notify_one();
		m_t.join();
	}
}

half& LightCalculator::lightValue(int x, int y)
{
	return m_map[y * m_snap_width * WORLD_CHUNK_SIZE + x];
}

half& LightCalculator::buffValue(int x, int y, half* buff)
{
	return buff[y * m_snap_width * WORLD_CHUNK_SIZE + x];
}

void LightCalculator::buffClear(half* buff, int cx, int cy)
{
	for (int y = 0; y < WORLD_CHUNK_SIZE; ++y)
	{
		memset(buff + y * m_snap_width * WORLD_CHUNK_SIZE + cx * WORLD_CHUNK_SIZE, 0, WORLD_CHUNK_SIZE);
	}
}

half& LightCalculator::lightValueSky(int x, int y)
{
	return m_map_sky[y * m_snap_width * WORLD_CHUNK_SIZE + x];
}

template <int DefaultVal>
half LightCalculator::getBlockOpacity(int x, int y)
{
	auto b = m_world->getBlockM(x, y);
	if (b)
		return BlockRegistry::get().getBlock(b->block_id).getOpacity();
	return DefaultVal; //outside map -> no light
}
template <int DefaultVal>
half LightCalculator::getBlockOpacity(int x, int y,ChunkPack& res)
{
	auto b = res.getBlockM(x, y);
	if (b)
		return BlockRegistry::get().getBlock(b->block_id).getOpacity();
	return DefaultVal; //outside map -> no light
}

template <uint8_t DefaultValue>
uint8_t& LightCalculator::blockLightLevel(int x, int y)
{
	static uint8_t defaul = DefaultValue;
	//todo this lasagna is causing problems because it is seen as light src on boundary ...we know
	//just to prevent problems with not loaded chunks/return so high light that no other updates will be neccessary
	auto b = m_world->getChunkM(x >> WORLD_CHUNK_BIT_SIZE, y >> WORLD_CHUNK_BIT_SIZE);
	if (b)
		return b->lightLevel(x & (WORLD_CHUNK_SIZE - 1), y & (WORLD_CHUNK_SIZE - 1));
	return defaul; //outside map -> no light
}
template <uint8_t DefaultValue>
uint8_t& LightCalculator::blockLightLevel(int x, int y,ChunkPack& res)
{
	static uint8_t defaul = DefaultValue;
	//todo this lasagna is causing problems because it is seen as light src on boundary ...we know
	//just to prevent problems with not loaded chunks/return so high light that no other updates will be neccessary
	auto b = res.getChunkM(x >> WORLD_CHUNK_BIT_SIZE, y >> WORLD_CHUNK_BIT_SIZE);
	if (b)
		return b->lightLevel(x & (WORLD_CHUNK_SIZE - 1), y & (WORLD_CHUNK_SIZE - 1));
	return defaul; //outside map -> no light
}

void LightCalculator::runFloodLocal(int minX, int minY, int width, int height,
                                    Utils::FifoList<Pos>* current_list, Utils::FifoList<Pos>* new_list)
{
	new_list->clear();
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
			auto opacity = getBlockOpacity(minX + x, minY + y);
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
		current_list->clear();
		auto t = current_list;
		current_list = new_list;
		new_list = t;
	}
}

void LightCalculator::runFloodSky(int minX, int minY, int width, int height,
                                  Utils::FifoList<Pos>* current_list, Utils::FifoList<Pos>* new_list)
{
	new_list->clear();
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

			auto val = lightValueSky(x, y);
			auto opacity = getBlockOpacity(minX + x, minY + y);
			half newLightPower = val - opacity;

			if (val < opacity) //we had overflow -> dark is coming for us all
				continue;

			//left
			int xm1 = x - 1;
			if (xm1 >= 0)
			{
				half& v = lightValueSky(xm1, y);
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
				half& v = lightValueSky(x, ym1);
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
				half& v = lightValueSky(x1, y);
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
				half& v = lightValueSky(x, y1);
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

void LightCalculator::computeChunkLT(int cx, int cy) //will be called on chunkgeneration
{
	auto& c = *m_world->getChunkM(cx, cy);
	auto current_list = &m_light_list0;
	auto new_list = &m_light_list1;

	current_list->clear();
	new_list->clear();
	//todo maybe lock chunk to prevent main thread from unloading it while working

	auto& biome = BiomeRegistry::get().getBiome(c.getBiome());
	uint8_t backLight = biome.getBackgroundLight(m_world);
	bool hasSky = biome.hasSkyLighting();
	//set whole chunk to dark and find lightsources

	for (int x = 0; x < WORLD_CHUNK_SIZE; ++x)
	{
		for (int y = 0; y < WORLD_CHUNK_SIZE; ++y)
		{
			if (backLight == 0)
			{
				auto& b = c.block(x, y);
				uint8_t val = BlockRegistry::get().getBlock(b.block_id).getLightSrcVal();
				c.lightLevel(x, y) = val;
				if (val == 0)
					continue;
				current_list->push({x, y});
			}
			else
			{
				auto& b = c.block(x, y);
				auto& bl = BlockRegistry::get().getBlock(b.block_id);
				uint8_t val = bl.getLightSrcVal();
				//check if ambient light will be there
				if (!hasSky && bl.getOpacity() <= 2 && val < backLight && (b.isWallFree() || BlockRegistry::get()
				                                                                             .getWall(b.wallID()).
				                                                                             isTransparent()))
				{
					c.lightLevel(x, y) = backLight;
					current_list->push({x, y});
				}
				else
				{
					c.lightLevel(x, y) = val;
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

			auto val = c.lightLevel(x, y);

			auto opacity = BlockRegistry::get().getBlock(c.block(x, y).block_id).getOpacity();
			half newLightPower = val - opacity;

			if (val < opacity) //we had overflow -> dark is coming for us all
				continue;

			//left
			int xm1 = x - 1;
			if (xm1 >= 0)
			{
				half& v = c.lightLevel(xm1, y);
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
				half& v = c.lightLevel(x, ym1);
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
				half& v = c.lightLevel(x1, y);
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
				half& v = c.lightLevel(x, y1);
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

void LightCalculator::computeChunkBordersLT(int cx, int cy,ChunkPack& res)
{
	auto& c = *res.getChunkM(cx, cy);

	auto current_list = &m_light_list0_main_thread;
	auto new_list = &m_light_list1_main_thread;

	current_list->clear();
	new_list->clear();

	int minX = cx * WORLD_CHUNK_SIZE;
	int minY = cy * WORLD_CHUNK_SIZE;

	Chunk* up =		res.getChunkM(cx, cy + 1);
	Chunk* down =	res.getChunkM(cx, cy - 1);
	Chunk* left =	res.getChunkM(cx - 1, cy);
	Chunk* right =	res.getChunkM(cx + 1, cy);
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

			auto val = blockLightLevel(x, y, res);
			auto opacity = getBlockOpacity(x, y,res);
			half l = val - opacity;

			if (val < opacity) //we had overflow -> dark is coming for us all
				continue;

			half newLightPower = l;

			//left
			int xm1 = x - 1;

			half* v = &blockLightLevel(xm1, y, res);
			if (*v < newLightPower)
			{
				*v = newLightPower;
				new_list->push({xm1, y});
			}

			//down
			int ym1 = y - 1;
			v = &blockLightLevel(x, ym1, res);
			if (*v < newLightPower)
			{
				*v = newLightPower;
				new_list->push({x, ym1});
			}
			//right
			int x1 = x + 1;
			v = &blockLightLevel(x1, y, res);
			if (*v < newLightPower)
			{
				*v = newLightPower;
				new_list->push({x1, y});
			}
			//up
			int y1 = y + 1;
			v = &blockLightLevel(x, y1, res);
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

LightCalculator::ChunkQuadro LightCalculator::createQuadroSquare(int wx, int wy)
{
	ChunkQuadro out;
	int chunkX = wx >> WORLD_CHUNK_BIT_SIZE;
	int chunkY = wy >> WORLD_CHUNK_BIT_SIZE;
	if (wx < WORLD_CHUNK_SIZE / 2)
	{
		if (wy < WORLD_CHUNK_SIZE / 2)
		{
			out[0] = std::make_pair(chunkX, chunkY);
			out[1] = std::make_pair(chunkX - 1, chunkY);
			out[2] = std::make_pair(chunkX, chunkY - 1);
			out[3] = std::make_pair(chunkX - 1, chunkY - 1);
		}
		else
		{
			out[0] = std::make_pair(chunkX, chunkY + 1);
			out[1] = std::make_pair(chunkX - 1, chunkY + 1);
			out[2] = std::make_pair(chunkX, chunkY);
			out[3] = std::make_pair(chunkX - 1, chunkY);
		}
	}
	else
	{
		if (wy < WORLD_CHUNK_SIZE / 2)
		{
			out[0] = std::make_pair(chunkX + 1, chunkY);
			out[1] = std::make_pair(chunkX, chunkY);
			out[2] = std::make_pair(chunkX + 1, chunkY - 1);
			out[3] = std::make_pair(chunkX, chunkY - 1);
		}
		else
		{
			out[0] = std::make_pair(chunkX + 1, chunkY + 1);
			out[1] = std::make_pair(chunkX, chunkY + 1);
			out[2] = std::make_pair(chunkX + 1, chunkY);
			out[3] = std::make_pair(chunkX, chunkY);
		}
	}
	return out;
}

LightCalculator::ChunkQuadro LightCalculator::createQuadroCross(int cx, int cy)
{
	ChunkQuadro out;

	out[0] = std::make_pair(cx, cy);
	out[1] = std::make_pair(cx - 1, cy);
	out[2] = std::make_pair(cx + 1, cy);
	out[3] = std::make_pair(cx, cy - 1);
	out[4] = std::make_pair(cx, cy + 1);
	return out;
}

void LightCalculator::computeChangeLT(int minX, int minY, int maxX, int maxY, int xx, int yy)
{
	//oughta be WORLD_CHUNK_SIZE / 2
	constexpr int maxLightRadius = 17; //true refresh area is one less
	auto current_list = &m_light_list0;
	auto new_list = &m_light_list1;

	auto current_list_sky = &m_light_list2;
	auto new_list_sky = &m_light_list3;

	current_list->clear();
	new_list->clear();

	current_list_sky->clear();
	new_list_sky->clear();
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
			bool isRendered = worldX >= minX && worldX < maxX && worldY >= minY && worldY < maxY;

			if (y == -yrad || y == yrad)
			{
				//todo maybe check if lightsrcval == 0
				current_list->push({worldX, worldY}); //add external boundary light
				if (isRendered)
					current_list_sky->push({worldX - minX, worldY - minY});
				continue;
			}

			int cx = worldX >> WORLD_CHUNK_BIT_SIZE;
			int cy = worldY >> WORLD_CHUNK_BIT_SIZE;
			Chunk* c = m_world->getChunkM(cx, cy);
			if (c == nullptr)
				continue;

			auto& biome = BiomeRegistry::get().getBiome(c->getBiome());
			uint8_t backLight = biome.getBackgroundLight(m_world);

			int chunkX = worldX & (BIT(WORLD_CHUNK_BIT_SIZE) - 1);
			int chunkY = worldY & (BIT(WORLD_CHUNK_BIT_SIZE) - 1);

			auto& b = c->block(chunkX, chunkY);

			auto& bl = BlockRegistry::get().getBlock(b.block_id);
			uint8_t val = bl.getLightSrcVal();

			bool isTransparentBlock = bl.getOpacity() <= 2 && (b.isWallFree() || BlockRegistry::get()
			                                                                     .getWall(b.wallID()).isTransparent());
			auto& skyValueRef = lightValueSky(worldX - minX, worldY - minY);
			skyValueRef = 0; //set to zero to clear previous data

			if (biome.hasSkyLighting() && isTransparentBlock)
			{
				skyValueRef = backLight;
				backLight = 0; //forbid to add this light down to cached light
				current_list_sky->push({worldX - minX, worldY - minY});
			}

			if (isTransparentBlock && backLight > val)
			{
				c->lightLevel(chunkX, chunkY) = backLight;
				if (backLight)
					current_list->push({worldX, worldY});
			}
			else
			{
				c->lightLevel(chunkX, chunkY) = val;
				if (val)
					current_list->push({worldX, worldY});
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

			auto val = blockLightLevel<0>(x, y);
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

	runFloodSky(minX, minY, maxX - minX, maxY - minY, current_list_sky, new_list_sky);
}

static struct SnapshotDim
{
	int offsetX, offsetY;
	int chunkWidth, chunkHeight;
} lastDim;

void LightCalculator::computeLT(Snapshot& sn)
{
	int minX = sn.offsetX * WORLD_CHUNK_SIZE;
	int minY = sn.offsetY * WORLD_CHUNK_SIZE;

	int width = sn.chunkWidth * WORLD_CHUNK_SIZE;
	int height = sn.chunkHeight * WORLD_CHUNK_SIZE;

	int maxX = minX + width;
	int maxY = minY + height;

	if (sn.chunkHeight != lastDim.chunkHeight
		|| sn.chunkWidth != lastDim.chunkWidth
		|| sn.offsetX != lastDim.offsetX
		|| sn.offsetY != lastDim.offsetY
	)
	{
		//clear everything

		lastDim.chunkWidth = sn.chunkWidth;
		lastDim.chunkHeight = sn.chunkHeight;
		lastDim.offsetX = sn.offsetX;
		lastDim.offsetY = sn.offsetY;

		m_light_list0.clear();
		m_light_list1.clear();

		auto current_list = &m_light_list0;
		auto new_list = &m_light_list1;

		memset(m_map_sky, 0, sn.chunkWidth * sn.chunkHeight * WORLD_CHUNK_AREA * sizeof(half));

		for (int cx = 0; cx < sn.chunkWidth; ++cx)
		{
			for (int cy = 0; cy < sn.chunkHeight; ++cy)
			{
				auto c = m_world->getChunk(cx + sn.offsetX, cy + sn.offsetY);
				if (c == nullptr)
					continue;
				auto& biome = BiomeRegistry::get().getBiome(c->getBiome());
				if (!biome.hasSkyLighting())
					continue;
				uint8_t backLight = biome.getBackgroundLight(m_world);
				for (int y = 0; y < WORLD_CHUNK_SIZE; ++y)
				{
					for (int x = 0; x < WORLD_CHUNK_SIZE; ++x)
					{
						auto& b = c->block(x, y);
						auto& bl = BlockRegistry::get().getBlock(b.block_id);
						if (bl.getOpacity() <= 2 && (b.isWallFree() || BlockRegistry::get()
						                                               .getWall(b.wallID()).isTransparent()))
						{
							auto xx = cx * WORLD_CHUNK_SIZE + x;
							auto yy = cy * WORLD_CHUNK_SIZE + y;
							lightValueSky(xx, yy) = backLight;
							current_list->push({xx, yy});
						}
					}
				}
			}
		}
		runFloodSky(minX, minY, width, height, current_list, new_list);
	}

	//first we need to update all cached light because that has priority
	m_cached_light_assign_mutex.lock();
	bool goOn = !m_cached_light_assignments.empty();
	m_cached_light_assign_mutex.unlock();

	std::set<std::pair<int, int>> acquiredResources;

	while (goOn)
	{
		m_cached_light_assign_mutex.lock();

		auto pop = m_cached_light_assignments.front();
		m_cached_light_assignments.pop();
		goOn = !m_cached_light_assignments.empty();

		m_cached_light_assign_mutex.unlock();
		switch (pop.type)
		{
		case Assignment::CHANGE:
			{
				auto res = createQuadroSquare(pop.x, pop.y);
				computeChangeLT(minX, minY, maxX, maxY, pop.x, pop.y);
				res.markDoneLightJob(*m_world);
			}
			break;
		case Assignment::CHUNK:
			{
				computeChunkLT(pop.x, pop.y);
				auto ccc = m_world->getChunkM(pop.x, pop.y);
				ccc->getLightJob().markDone();
			}
			break;
		case Assignment::BORDERS:
			{
				computeChunkBordersLT(pop.x, pop.y,pop.res);
				pop.res.markLightJobDone();
			}
			break;
		}
	}
	memcpy(m_map_sky_out, m_map_sky, sn.chunkWidth * sn.chunkHeight * WORLD_CHUNK_AREA * sizeof(half));

	updateLocalMapLT(sn); //set map content to cached chunk light


	//add dynamic lighting
	m_light_list0.clear();
	m_light_list1.clear();

	auto current_list = &m_light_list0;
	auto new_list = &m_light_list1;


	//add dynamic light sources to map
	for (auto& light : sn.data)
	{
		if (light.x >= minX && light.x < maxX && light.y >= minY && light.y < maxY)
		{
			auto& l = lightValue(light.x - minX, light.y - minY);
			if (l < light.intensity)
			{
				l = light.intensity;
				current_list->push({light.x - minX, light.y - minY});
			}
		}
	}
	runFloodLocal(minX, minY, width, height, current_list, new_list);
}

void LightCalculator::updateLocalMapLT(Snapshot& sn)
{
	//todo this needs to use memcpy or death will feast upon us all
	for (int cx = 0; cx < sn.chunkWidth; ++cx)
		for (int cy = 0; cy < sn.chunkHeight; ++cy)
		{
			auto c = m_world->getChunk(cx + sn.offsetX, cy + sn.offsetY);
			if (c)
			{
				for (int x = 0; x < WORLD_CHUNK_SIZE; ++x)
					for (int y = 0; y < WORLD_CHUNK_SIZE; ++y)
						lightValue(cx * WORLD_CHUNK_SIZE + x, cy * WORLD_CHUNK_SIZE + y) = c->lightLevel(x, y);
			}
			else
				for (int x = 0; x < WORLD_CHUNK_SIZE; ++x)
					for (int y = 0; y < WORLD_CHUNK_SIZE; ++y)
						lightValue(cx * WORLD_CHUNK_SIZE + x, cy * WORLD_CHUNK_SIZE + y) = 0;
		}
}

void LightCalculator::darkenLT(Snapshot& sn) //deprecated
{
	//todo this needs to use memcpy or death will feast upon us all
	for (int cx = 0; cx < sn.chunkWidth; ++cx)
		for (int cy = 0; cy < sn.chunkHeight; ++cy)
		{
			auto c = m_world->getChunkM(cx + sn.offsetX, cy + sn.offsetY);
			if (c)
				for (int y = 0; y < WORLD_CHUNK_SIZE; ++y)
					for (int x = 0; x < WORLD_CHUNK_SIZE; ++x)
						c->lightLevel(x, y) = 0;
		}
}

void LightCalculator::setDimensionsInnerLT()
{
	if (m_big_buffer)
		delete[] m_big_buffer;

	size_t oneSize = m_snap_width * m_snap_height * WORLD_CHUNK_AREA;

	m_big_buffer = new half[oneSize * 5];

	m_map = m_big_buffer;
	m_map_done = m_big_buffer + oneSize * 1;
	m_map_sky = m_big_buffer + oneSize * 2;
	m_map_sky_out = m_big_buffer + oneSize * 3;
	m_map_sky_out_done = m_big_buffer + oneSize * 4;

	/*m_map_done = new half[oneSize];
	m_map_sky = new half[oneSize];
	m_map_sky_out = new half[oneSize];
	m_map_sky_out_done = new half[oneSize];*/
}
