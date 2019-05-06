#pragma once
#include "ndpch.h"
#include "block/Block.h"
class BlockRegistry
{

private:
	std::vector<Block*> m_blocks;
	BlockRegistry();


public:
	~BlockRegistry();

	inline int size(){return m_blocks.size();}
	inline  const std::vector<Block*>& getBlocks() { return m_blocks; }


	static inline BlockRegistry& get() { 
		static BlockRegistry s_instance;
		return s_instance;
	}
	BlockRegistry(BlockRegistry const&) = delete;
	void operator=(BlockRegistry const&) = delete;
public:
	void registerBlock(Block* block);

	const Block& getBlock(int block_id);

};
