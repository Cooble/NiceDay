#pragma once
#include "Block.h" 

class BlockAir:public Block
{
public:
	BlockAir();
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	int getCornerOffset(int x, int y, const BlockStruct&) const override;
	bool onNeighbourBlockChange(World* world, int x, int y) const override;
	
	TO_STRING(BlockAir)
};
class BlockStone :public Block
{
public:
	BlockStone();
	TO_STRING(BlockStone)
};
class BlockDirt :public Block
{
public:
	BlockDirt();
	TO_STRING(BlockDirt)
};
class BlockGold :public Block
{
public:
	BlockGold();
	TO_STRING(BlockGold)
};
class BlockAdamantite :public Block
{
public:
	BlockAdamantite();
	TO_STRING(BlockAdamantite)
};
class BlockPlatform :public Block
{
public:
	BlockPlatform();
	bool onNeighbourBlockChange(World* world, int x, int y) const override;
	int getCornerOffset(int x, int y, const BlockStruct&) const override;
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	TO_STRING(BlockPlatform)
};
class BlockGrass :public Block
{
public:
	BlockGrass();
	bool onNeighbourBlockChange(World* world, int x, int y) const override;
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	TO_STRING(BlockGrass)
};
class BlockGlass :public Block
{
public:
	BlockGlass();
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	bool onNeighbourBlockChange(World* world, int x, int y) const override;
	TO_STRING(BlockGlass)
};
class BlockTorch :public Block
{
public:
	BlockTorch();
	TO_STRING(BlockTorch)
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	int getCornerOffset(int x, int y, const BlockStruct&) const override;
	bool isInTorchGroup(World* world, int x, int y)const;
	bool onNeighbourBlockChange(World* world, int x, int y) const override;
};
