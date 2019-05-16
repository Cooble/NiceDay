#pragma once
#include "Block.h"

class BlockAir:public Block
{
public:
	BlockAir();
	BLOCK_TO_STRING(BlockAir)
};
class BlockStone :public Block
{
public:
	BlockStone();
	BLOCK_TO_STRING(BlockStone)
};
class BlockDirt :public Block
{
public:
	BlockDirt();
	BLOCK_TO_STRING(BlockDirt)
};
class BlockGold :public Block
{
public:
	BlockGold();
	BLOCK_TO_STRING(BlockGold)
};
class BlockAdamantite :public Block
{
public:
	BlockAdamantite();
	BLOCK_TO_STRING(BlockAdamantite)
};
