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
	std::unordered_map<std::string, int> m_blockIDs;
	std::unordered_map<std::string, int> m_wallIDs;
	std::vector<Block*> m_blocks;
	std::vector<Wall*> m_walls;
	BlockRegistry()=default;


public:
	BlockRegistry(BlockRegistry const&) =delete;
	void operator=(BlockRegistry const&)=delete;
	~BlockRegistry();

	inline const std::vector<Block*>& getBlocks() { return m_blocks; }
	inline const std::vector<Wall*>& getWalls() { return m_walls; }

	void initTextures(const TextureAtlas& atlas);

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

	const Block& getBlock(const std::string& block_id) const;

	const Wall& getWall(const std::string& wall_id) const;


	inline int getBlockID(const std::string& block_id) const { return getBlock(block_id).getID(); }
	inline int getWallID(const std::string& wall_id) const { return getWall(wall_id).getID(); }
};

