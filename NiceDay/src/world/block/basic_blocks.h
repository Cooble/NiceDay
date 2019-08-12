#pragma once
#include "Block.h" 

class BlockAir:public Block
{
public:
	BlockAir();
	inline void onTextureLoaded(const TextureAtlas& atlas) override{}
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	int getCornerOffset(int x, int y, const BlockStruct&) const override;
	bool onNeighbourBlockChange(World* world, int x, int y) const override;
	
	UUID_STRING("air")
};
class BlockStone :public Block
{
public:
	BlockStone();
	UUID_STRING("stone")
};
class BlockDirt :public Block
{
public:
	BlockDirt();
	UUID_STRING("dirt")
};
class BlockGold :public Block
{
public:
	BlockGold();
	UUID_STRING("gold")
};
class BlockAdamantite :public Block
{
public:
	BlockAdamantite();
	UUID_STRING("adamantite")
};
class BlockPlatform :public Block
{
public:
	BlockPlatform();
	bool onNeighbourBlockChange(World* world, int x, int y) const override;
	int getCornerOffset(int x, int y, const BlockStruct&) const override;
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	UUID_STRING("platform")
};
class BlockGrass :public Block
{
public:
	BlockGrass();
	bool onNeighbourBlockChange(World* world, int x, int y) const override;
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	UUID_STRING("grass")
};
class BlockGlass :public Block
{
public:
	BlockGlass();
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	bool onNeighbourBlockChange(World* world, int x, int y) const override;
	UUID_STRING("glass")
};
class BlockTorch :public Block
{
public:
	BlockTorch();
	UUID_STRING("torch")
	int getTextureOffset(int x, int y, const BlockStruct&) const override;
	int getCornerOffset(int x, int y, const BlockStruct&) const override;
	bool isInTorchGroup(World* world, int x, int y)const;
	bool onNeighbourBlockChange(World* world, int x, int y) const override;
};
class BlockDoor :public MultiBlock
{
public:
	BlockDoor(int id);
	void onBlockClicked(World* w, WorldEntity* e, int x, int y, BlockStruct& curBlok) const override;
};
class BlockDoorOpen :public BlockDoor
{
public:
	BlockDoorOpen();
	UUID_STRING("door_open")
	void onTextureLoaded(const TextureAtlas& atlas) override;
};
class BlockDoorClose :public BlockDoor
{
public:
	BlockDoorClose();
	UUID_STRING("door_close")
	bool canBePlaced(World* w, int x, int y) const override;
	void onTextureLoaded(const TextureAtlas& atlas) override;
};
class BlockPainting :public MultiBlock
{
public:
	BlockPainting();
	UUID_STRING("painting")
	bool canBePlaced(World* w, int x, int y) const override;
};

class BlockTree :public Block
{
	 
public:
	BlockTree();
	UUID_STRING("tree")
	//int getTextureOffset(int x, int y, const BlockStruct&) const override;
	int getCornerOffset(int x, int y, const BlockStruct&) const override;
	bool onNeighbourBlockChange(World* world, int x, int y) const override;
};
