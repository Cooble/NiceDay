#pragma once
#include "ndpch.h"

#include "block/BlockRegistry.h"



#define MAX_SNAPSHOT_NUMBER 5

class World;
class LightSource
{
public:
	virtual std::pair<int, int> getLightPosition() const = 0;//position in blocks
	virtual float getIntensity() const = 0;
	virtual ~LightSource() = default;
};

typedef float half;//maybe in future replace with half float to save space
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


private:
	volatile bool m_running=false;
	volatile bool m_is_fresh_map=false;
	struct LightData
	{
		float intensity;
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
	volatile int m_chunk_width=0, m_chunk_height=0;

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
	inline float getBlockOpacity(int x, int y);

	void computeLight(Snapshot& snapshot);
	void runInner();
	void setDimensionsInner();
	
};
