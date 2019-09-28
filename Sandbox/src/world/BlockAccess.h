#pragma once
struct BlockStruct;
class Chunk;
class BlockAccess
{
public:
	virtual ~BlockAccess() = default;

	//returns mutable pointer to valid loaded block or nullptr
	virtual BlockStruct* getBlockM(int x, int y) = 0;
	virtual Chunk* getChunkM(int cx, int cy) = 0;
	virtual void setBlock(int x, int y, BlockStruct& block) = 0;
	void setBlock(int x, int y, int blockid);
	virtual void setBlockWithNotify(int x, int y, BlockStruct& block) = 0;
	void setBlockWithNotify(int x, int y, int blockid);
	virtual void setWall(int x, int y, int wallid) = 0;
};

