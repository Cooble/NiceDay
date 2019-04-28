#pragma once
#include "world/World.h"
#include "entity/Camera.h"
#include "world/ITickable.h"
#include <glm/vec2.hpp>


class IChunkLoaderEntity {
public:
	/*Location in world*/
	virtual const glm::vec2& getPosition() const = 0;
	/*Return 0 if no chunk should be updated*/
	virtual int getChunkRadius() const = 0;
};

struct EntityWrapper{
	IChunkLoaderEntity* e;
	int last_chunk_id;
	int last_chunk_radius;


};
class ChunkLoader
{
private:
	World* m_world;
	std::vector<EntityWrapper> m_loader_entities;
	void tickInner();

public:
	ChunkLoader(World* w);
	~ChunkLoader();


	void onUpdate();

	void registerEntity(IChunkLoaderEntity* e);
	void unregisterEntity(IChunkLoaderEntity* e);
};

