#pragma once
#include "ndpch.h"
#include "Block.h"

#define ND_REGISTER_BLOCK(block)\
	BlockRegistry::get().registerBlock(block);
#define ND_REGISTER_WALL(wall)\
	BlockRegistry::get().registerWall(wall);
class BlockRegistry
{

private:
	std::vector<Block*> m_blocks;
	std::vector<Wall*> m_walls;
	BlockRegistry()=default;


public:
	BlockRegistry(BlockRegistry const&) =delete;
	void operator=(BlockRegistry const&)=delete;
	~BlockRegistry();

	inline  const std::vector<Block*>& getBlocks() { return m_blocks; }
	inline  const std::vector<Wall*>& getWalls() { return m_walls; }


	static inline BlockRegistry& get() { 
		static BlockRegistry s_instance;
		return s_instance;
	}
public:
	//takes ownership
	void registerBlock(Block* block);
	//takes ownership
	void registerWall(Wall* wall);

	const Block& getBlock(int block_id);

	const Wall& getWall(int wall_id);

};

