﻿#pragma once
#include "Block.h"

class BlockTextureAtlas;

class BlockAir : public Block
{
public:
	BlockAir();

	inline void onTextureLoaded(const BlockTextureAtlas& atlas) override
	{
	}

	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	int getCornerOffset(int x, int y, const BlockStruct&) const override;
	bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override;

	UUID_STRING("air")
};

class BlockStone : public Block
{
public:
	BlockStone();
	UUID_STRING("stone")
};

class BlockDirt : public Block
{
public:
	BlockDirt();
	UUID_STRING("dirt")
};

class BlockGold : public Block
{
public:
	BlockGold();
	UUID_STRING("gold")
};

class BlockAdamantite : public Block
{
public:
	BlockAdamantite();
	UUID_STRING("adamantite")
};

class BlockPlatform : public Block
{
public:
	BlockPlatform();
	bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override;
	int getCornerOffset(int x, int y, const BlockStruct&) const override;
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	UUID_STRING("platform")
};

class BlockGrass : public Block
{
public:
	BlockGrass();
	bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override;
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	UUID_STRING("grass")
};

class BlockGlass : public Block
{
public:
	BlockGlass();
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override;
	UUID_STRING("glass")
};

class BlockTorch : public Block
{
public:
	BlockTorch();
	UUID_STRING("torch")
	void onBlockClicked(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const override;
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	int getCornerOffset(int x, int y, const BlockStruct&) const override;
	bool isInTorchGroup(BlockAccess& world, int x, int y) const;
	bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override;
};

class BlockDoor : public MultiBlock
{
public:
	BlockDoor(int id);
	void onBlockClicked(World& w, WorldEntity* e, int x, int y, BlockStruct& curBlok) const override;
};

class BlockDoorOpen : public BlockDoor
{
public:
	BlockDoorOpen();
	UUID_STRING("door_open")
	void onTextureLoaded(const BlockTextureAtlas& atlas) override;
};

class BlockDoorClose : public BlockDoor
{
public:
	BlockDoorClose();
	UUID_STRING("door_close")
	bool canBePlaced(World& w, int x, int y) const override;
	void onTextureLoaded(const BlockTextureAtlas& atlas) override;
};

class BlockPainting : public MultiBlock
{
public:
	BlockPainting();
	UUID_STRING("painting")
	bool canBePlaced(World& w, int x, int y) const override;
};
class BlockTreeSapling :public Block
{
public:
	BlockTreeSapling();
	UUID_STRING("sapling")
	bool canBePlaced(World& w, int x, int y) const override;
	int getCornerOffset(int x, int y, const BlockStruct&) const override;
};

class BlockTree : public Block
{
protected:
	half_int m_texture_map[20];
public:
	BlockTree();
	UUID_STRING("tree")
	void onTextureLoaded(const BlockTextureAtlas& atlas) override;
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	int getCornerOffset(int x, int y, const BlockStruct&) const override;
	bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override;
	void onBlockDestroyed(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const override;
};
namespace BlockTreeScope
{
	constexpr int RootL = 0;
	constexpr int RootR = 1;
	constexpr int fullTrunk2W = 2;

	constexpr int dryBranchL = 3;
	constexpr int dryBranchR = 4;

	constexpr int branchL = 5;
	constexpr int branchR = 6;

	constexpr int trunkBranchL = 7;
	constexpr int trunkBranchR = 8;

	constexpr int smallBlobL2W = 9;
	constexpr int smallBlobR2W = 10;

	constexpr int thinTrunkL = 11;
	constexpr int thinTrunkR = 12;

	constexpr int dryBlobL = 13;
	constexpr int dryBlobR = 14;

	constexpr int bigBlob0 = 15;
	constexpr int bigBlob1 = 16;
	constexpr int blob = 17;

	constexpr int trunkL = 18;
	constexpr int trunkR = 19;
};

class BlockPlant :public Block
{
protected:
	int m_max_metadata;
public:
	BlockPlant(int id);
	int getTextureOffset(int x, int y, const BlockStruct& b) const override;
	inline bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override { return false; }
	inline int getCornerOffset(int x, int y, const BlockStruct& b) const override { return BLOCK_STATE_FULL; }
	bool canBePlaced(World& w, int x, int y) const override;
	void onBlockPlaced(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const override;

};
class BlockFlower :public BlockPlant
{
public:
	BlockFlower();
	UUID_STRING("flower")
};
class BlockGrassPlant :public BlockPlant
{
public:
	BlockGrassPlant();
	UUID_STRING("grass_plant")
};
