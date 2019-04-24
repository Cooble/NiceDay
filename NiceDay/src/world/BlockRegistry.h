#pragma once
#include "ndpch.h"
#include "Block.h"
class BlockRegistry
{

private:
	std::vector<Block> m_blocks;
	BlockRegistry();


public:
	~BlockRegistry();
	static inline BlockRegistry& get() { 
		static BlockRegistry s_instance;
		return s_instance;
	}
	BlockRegistry(BlockRegistry const&) = delete;
	void operator=(BlockRegistry const&) = delete;
public:
	void registerBlock(Block& block);

	const Block& getBlock(int block_id);

};

