#pragma once
#include "world/World.h"
#include "entity/I2DLocalable.h"


class IChunkLoaderEntity:public I2DLocalable 
{
public:
	virtual ~IChunkLoaderEntity() = default;
	/*Return 0 if no chunk should be updated*/
	virtual half_int getChunkRadius() const = 0;
};

struct EntityWrapper{
	IChunkLoaderEntity* e;
	int last_chunk_id;
	half_int last_chunk_radius;


};
class ChunkLoader
{
private:
	World* m_world;
	bool m_dirty;//entity was added need to recalculate chunks
	std::vector<EntityWrapper> m_loader_entities;
	void tickInner();

public:
	ChunkLoader(World* w);
	~ChunkLoader();

	
	void onUpdate();


	void registerEntity(IChunkLoaderEntity* e);
	void unregisterEntity(IChunkLoaderEntity* e);
	
	/*unregisters all entities need to call onUpdate() to unload all chunks*/
	void clearEntities();
};

