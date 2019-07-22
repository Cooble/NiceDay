#pragma once
#include "ndpch.h"

#include "block/BlockRegistry.h"
#include "WorldGen.h"


#define MAX_SNAPSHOT_NUMBER 5

class World;
class LightSource
{
public:
	virtual std::pair<int, int> getLightPosition() const = 0;//position in blocks
	virtual uint8_t getIntensity() const = 0;
	virtual ~LightSource() = default;
};

typedef uint8_t half;//maybe in future replace with half float to save space
typedef std::pair<int, int> ChunkPos;
class World;

class LightCalculator
{
public:
	LightCalculator(World* world);
	~LightCalculator();

	void setDimensions(int chunkWidth, int chunkHeight);//change in dimensions will dump all snapshots on light thread
	void setChunkOffset(int x, int y);


	void run();//makes a new light computing thread and returns back
	inline bool isRunning() const { return m_running; }
	inline bool isFreshMap()
	{
		if (m_is_fresh_map)
		{
			m_is_fresh_map = false;
			return true;
		}
		return false;
	}

	void stop();//stops light calculating thread

	void snapshot();//saves current light states to be computed to lightmap later

	inline half* getCurrentLightMap() const { return m_done_map; }
	inline ChunkPos getCurrentOffset() const { return m_done_ch_offset; }


	void registerLight(LightSource* light); //it doesn't take ownership, need remove()
	void removeLight(LightSource* light);

	// clears all light data and recalculates the chunk lighting (without affecting or being affected by others)
	// (chunks passed should be locked) maybe
	void computeChunk(Chunk& c);
	// floods light from all external boundary blocks (block that are around the chunk but not within)
	// (chunks passed should be locked) maybe
	void computeChunkBorders(Chunk& c);

	struct ChunkQuadro
	{
		std::pair<int, int> src[4];

		std::pair<int, int>& operator[](size_t index)
		{
			return src[index];
		}

	};

	// returns 4 chunks that will be needed to proccess cached light change
	static ChunkQuadro computeQuadro(int wx, int wy);

	// recalculates all light within specific radius (=max light radius)
	void computeChange(int x, int y);

	// adds work for light thread and returns
	// recalculates all light within specific radius (=max light radius)
	// it is mandatory to load all needed resources before hand! and lock them
	// those resources will be marked unlocked after change
	void assignComputeChange(int x, int y);


private:
	std::condition_variable m_wait_condition_variable;

	struct Pos
	{
		int x, y;
	};
	std::mutex m_cached_light_assign_mutex;
	std::queue<Pos> m_cached_light_assignments;



	NDUtil::FifoList<Pos> m_light_list0;
	NDUtil::FifoList<Pos> m_light_list1;

	NDUtil::FifoList<Pos> m_light_list0_main_thread;
	NDUtil::FifoList<Pos> m_light_list1_main_thread;
	volatile bool m_running=false;
	volatile bool m_is_fresh_map=false;
	struct LightData
	{
		uint8_t intensity;
		int x, y;
	};
	struct Snapshot
	{
		std::vector<LightData> data;
		int offsetX,offsetY;
		int chunkWidth, chunkHeight;
	};
	//state set currently by main thread
	int m_chunk_offset_x, m_chunk_offset_y;

	//state set currently by main thread and read by light thread
	int m_chunk_width=0, m_chunk_height=0;

	//state owned by light thread
	int m_snap_width = 0, m_snap_height=0;

	World* m_world;

	//lights registered in calculator
	std::vector<LightSource*> m_sources;
	
	std::mutex m_snapshot_queue_mutex;
	//snapshots of lights to be drawn
	std::queue<Snapshot*> m_snapshot_queue;

	half* m_map;//is being written to (by another thread)
	half* m_done_map;//this is for render purposes

	ChunkPos m_done_ch_offset;

	inline half& lightValue(int x, int y);
	inline half getBlockOpacity(int x, int y);
	uint8_t& blockLightLevel(int x, int y);
	uint8_t& blockLightLevelDefault0(int x, int y);
	void computeLightOld(Snapshot& sn);


	void computeLight(Snapshot& snapshot);
	void runInner();
	void updateMap(Snapshot& sn);
	void darken(Snapshot& sn);
	void setDimensionsInner();
	
};
