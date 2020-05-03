#pragma once
#include "Block.h"

class BlockTextureAtlas;

class BlockAir : public Block
{
public:
	BlockAir();

	void onTextureLoaded(const BlockTextureAtlas& atlas) override {}

	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override;

};
class BlockPumpkin : public MultiBlock
{
public:
	BlockPumpkin();
	void onBlockPlaced(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const override;
};
class BlockChest : public MultiBlock
{
	half_int m_open_texture;
	half_int m_close_texture;
public:
	BlockChest();

	void onTextureLoaded(const BlockTextureAtlas& atlas) override;
	int getTextureOffset(int x, int y, const BlockStruct& b) const override;
	void openChest(World& w, int x,int y,bool open)const;
	bool isOpened(BlockStruct& b)const;

};

class BlockPlatform : public Block
{
public:
	BlockPlatform();
	bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override;
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
};

class BlockGrass : public Block
{
public:
	BlockGrass();
	bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override;
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
};

class BlockGlass : public Block
{
public:
	BlockGlass();
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override;
};

class BlockTorch : public Block
{
public:
	BlockTorch();
	void onBlockClicked(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const override;
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	bool isInTorchGroup(BlockAccess& world, int x, int y) const;
	bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override;
};

class BlockDoor : public MultiBlock
{
public:
	BlockDoor(std::string id);
	void onBlockClicked(World& w, WorldEntity* e, int x, int y, BlockStruct& curBlok) const override;
	std::string getItemIDFromBlock() const override;

};

class BlockDoorOpen : public BlockDoor
{
public:
	BlockDoorOpen();
	void onTextureLoaded(const BlockTextureAtlas& atlas) override;
};

class BlockDoorClose : public BlockDoor
{
public:
	BlockDoorClose();
	bool canBePlaced(World& w, int x, int y) const override;
	void onTextureLoaded(const BlockTextureAtlas& atlas) override;
};
class BlockTreeSapling :public Block
{
public:
	BlockTreeSapling();
	bool canBePlaced(World& w, int x, int y) const override;
};

class BlockTree : public Block
{
protected:
	half_int m_texture_map[20];
public:
	BlockTree();
	void onTextureLoaded(const BlockTextureAtlas& atlas) override;
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
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
public:
	BlockPlant(std::string id);
	int getTextureOffset(int x, int y, const BlockStruct& b) const override;
	bool onNeighbourBlockChange(BlockAccess& world, int x, int y) const override;
	bool canBePlaced(World& w, int x, int y) const override;
	void onBlockPlaced(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const override;

};

