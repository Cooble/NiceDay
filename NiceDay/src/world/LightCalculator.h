#pragma once
#include "ndpch.h"

#include "block/BlockRegistry.h"

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
	void stop();//stops light calculating thread
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


	void snapshot();//saves current light states to be computed to lightmap later

	inline half* getCurrentLightMap() const { return m_done_map; }
	inline half* getCurrentLightMapChunkBack() const { return m_done_map_sky_out; }
	inline ChunkPos getCurrentOffset() const { return m_done_ch_offset; }


	void registerLight(LightSource* light); //it doesn't take ownership, need remove()
	void removeLight(LightSource* light);

	struct ChunkQuadro
	{
		std::pair<int, int> src[4];

		std::pair<int, int>& operator[](size_t index)
		{
			return src[index];
		}

	};
	// returns 4 chunks that will be needed to proccess cached light change
	static ChunkQuadro computeQuadroSquare(int wx, int wy);
	// returns 4 chunks that will be needed to proccess cached light chunkborder change
	static ChunkQuadro createQuadroCross(int wx, int wy);

	// adds work for light thread and returns
	// don't forget to assign lightjob for all neighbour chunks as well with LightCalculator::computeQuadroSquare()
	// recalculates all cached light within specific radius (=max light radius)
	void assignComputeChange(int x, int y);

	// adds work for light thread and returns
	// don't forget to assign lightjob to chunk!
	// clears all light data and recalculates the chunk lighting (without affecting or being affected by others)
	void assignComputeChunk(int cx,int cy);

	// adds work for light thread and returns
	// floods light from all external boundary blocks (block that are around the chunk but not within)
	// don't forget to assign lightjob for all neighbour chunks as well with LightCalculator::createQuadroCross()
	void assignComputeChunkBorders(int cx,int cy);

private:
	std::condition_variable m_wait_condition_variable;

	 
	struct Assignment
	{
		int x, y;
		enum
		{
			CHANGE,
			CHUNK,
			BORDERS
		} type;
	};
	struct Pos
	{
		int x, y;
	};
	std::mutex m_cached_light_assign_mutex;
	std::queue<Assignment> m_cached_light_assignments;// Have I found everybody fun assignment to do today?



	NDUtil::FifoList<Pos> m_light_list0;
	NDUtil::FifoList<Pos> m_light_list1;

	NDUtil::FifoList<Pos> m_light_list2;
	NDUtil::FifoList<Pos> m_light_list3;

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

	half* m_big_buffer;
	half* m_map;//is being written to (by light thread)
	half* m_done_map;//swapchain

	half* m_map_sky;//is being written to (by light thread)
	half* m_map_sky_out;//swapchain0
	half* m_done_map_sky_out;//swapchain1

	ChunkPos m_done_ch_offset;

	inline half& lightValue(int x, int y);
	inline half& buffValue(int x, int y,half* buff);
	inline void buffClear(half* buff,int cx, int cy);
	inline half& lightValueSky(int x, int y);
	template<int DefaultVal=std::numeric_limits<half>::max()>
	inline half getBlockOpacity(int x, int y);
	template<uint8_t DefaultValue=0>
	inline uint8_t& blockLightLevel(int x, int y);

	// runs flood algorithm using worldblocks' opacity (without blocks' light level) and edits local map
	// depends only on		map, 
	//						world opacity,
	//						current_list
	void runFloodLocal(int minX, int minY, int width, int height, NDUtil::FifoList<Pos>* current_list, NDUtil::FifoList<Pos>* new_list);
	void runFloodSky(int minX, int minY, int width, int height, NDUtil::FifoList<Pos>* current_list,
	                       NDUtil::FifoList<Pos>* new_list);


	void computeLT(Snapshot& snapshot);
	void runInnerLT();
	void updateMapLT(Snapshot& sn);
	void darkenLT(Snapshot& sn);
	void setDimensionsInnerLT();

	// clears all light data and recalculates the chunk lighting (without affecting or being affected by others)
// (chunks passed should be locked) maybe
	void computeChunkLT(int cx,int cy);
	// floods light from all external boundary blocks (block that are around the chunk but not within)
	// (chunks passed should be locked) maybe
	void computeChunkBordersLT(int cx, int cy);



	// recalculates all light within specific radius (=max light radius)
	void computeChangeLT(int minX, int minY, int maxX, int maxY, int xx, int yy);


	
};
