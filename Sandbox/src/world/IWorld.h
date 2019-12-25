#pragma once
#include "ndpch.h"
#include "particle/ParticleManager.h"
#include "entity/EntityManager.h"
#include "memory/stack_allocator.h"

struct BlockStruct;
struct Chunk;

/**
 *  Theee Interface of theee world (ワールド)
 */
class IWorld
{
public:
	virtual ~IWorld() = default;

	//=================================BLOCKS=================================
	/**
	 *  @return block in the world or null if not loaded
	 */
	virtual const BlockStruct* getBlock(int wx, int wy) const=0;

	/**
	 *  @return block in the world or null if not loaded
	 *  @note: You can modify the block but no events regarding block change will be fired
	 */
	virtual BlockStruct* modifyBlock(int wx, int wy) =0;

	/**
	 *  @return chunk in the world or null if not loaded
	 */
	virtual const Chunk* getChunk(int cx, int cy) const = 0;

	/**
	 *  @return chunk in the world or null if not loaded
	 *  Note: You can modify blocks of the chunk but no events regarding block change will be fired
	 */
	virtual Chunk* modifyChunk(int cx, int cy) = 0;

	/**
	 * Sets the block and fire events regarding block change
	 */
	virtual void setBlock(int wx, int wy, const BlockStruct& block) = 0;

	/**
	 * Sets the wall and fire events regarding wall change
	 */
	virtual void setWall(int wx, int wy, const BlockStruct& wall) = 0;

	/**
	 * Sets the block and fire events regarding block change
	 */
	void setBlock(int wx, int wy, int blockid);
	/**
	 * Sets the wall and fire events regarding wall change
	 */
	void setWall(int wx, int wy, int wallid);

	//=================================PARTICLES==============================

	virtual void spawnParticle(ParticleID id, const glm::vec2& pos, const glm::vec2& speed, const glm::vec2& acc, int life, float rotation = 0) = 0;

	//=================================ENTITIES===============================

	/**
	 * Spawns entity inside the world and sets custom entity id to that entity
	 * @return newly generated entityID
	 */
	virtual EntityID spawnEntity(WorldEntity* pEntity)=0;

	/**
	 * kills (tile)entity no matter whether it is loaded or not
	 * @attention
	 * never call on yourself immediately calls ~().
	 * To kill yourself safely use entity.markDead() instead (~() will be called after update() of this entity)
	 * 
	 */
	virtual void killEntity(EntityID id)=0;

	/**
	 * @return entityPointer or null if not loaded
	 */
	virtual WorldEntity* getLoadedEntity(EntityID id);
	/**
	 * @return current list of loaded entities
	 */
	virtual const std::vector<EntityID>& getLoadedEntities() = 0;

	/**
	 *	@return all (tile)entities 
	 */
	virtual nd::temp_vector<EntityID> getEntitiesInRadius(const glm::vec2& centerPos, float radius) = 0;

	
};
