#pragma once
#include "Block.h"

class BlockAir:public Block
{
public:
	BlockAir();

	int getTextureOffset(int metadata) const override;
	BLOCK_TO_STRING(BlockAir);
	
};
class BlockStone :public Block
{
public:
	BlockStone();

	int getTextureOffset(int metadata) const override;
	BLOCK_TO_STRING(BlockStone);


};
class BlockDirt :public Block
{
public:
	BlockDirt();

	int getTextureOffset(int metadata) const override;
	BLOCK_TO_STRING(BlockDirt);

};
class BlockGold :public Block
{
public:
	BlockGold();

	int getTextureOffset(int metadata) const override;
	BLOCK_TO_STRING(BlockGold);
};
class BlockAdamantite :public Block
{
public:
	BlockAdamantite();

	int getTextureOffset(int metadata) const override;
	BLOCK_TO_STRING(BlockAdamantite);
};
