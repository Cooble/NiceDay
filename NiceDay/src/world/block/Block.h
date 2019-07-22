#pragma once
#include "ndpch.h"
#include "physShapes.h"
//STATES=========================================================
//is air counterclockwise start from up(=lsb) (up left down right)
constexpr int BLOCK_STATE_FULL = 0;
//		  ______________
//		 <              >
//		 >			    <
//		 <\/\/\/\/\/\/\/>
//
constexpr int BLOCK_STATE_LINE_UP = BIT(0);
constexpr int BLOCK_STATE_LINE_DOWN = BIT(2);
constexpr int BLOCK_STATE_LINE_LEFT = BIT(1);
constexpr int BLOCK_STATE_LINE_RIGHT = BIT(3);

//     ____
//    |      \     
//    |		    \
//    |__________|
//
constexpr int BLOCK_STATE_CORNER_UP_LEFT = (BLOCK_STATE_LINE_UP | BLOCK_STATE_LINE_LEFT);
constexpr int BLOCK_STATE_CORNER_UP_RIGHT = (BLOCK_STATE_LINE_UP | BLOCK_STATE_LINE_RIGHT);
constexpr int BLOCK_STATE_CORNER_DOWN_LEFT = (BLOCK_STATE_LINE_DOWN | BLOCK_STATE_LINE_LEFT);
constexpr int BLOCK_STATE_CORNER_DOWN_RIGHT = (BLOCK_STATE_LINE_DOWN | BLOCK_STATE_LINE_RIGHT);

constexpr int BLOCK_STATE_BIT = (BLOCK_STATE_CORNER_UP_LEFT | BLOCK_STATE_CORNER_DOWN_RIGHT);
//		  ______________
//		 <              >
//		 >			    <
//		 <______________>
//
constexpr int BLOCK_STATE_LINE_HORIZONTAL = (BLOCK_STATE_LINE_UP | BLOCK_STATE_LINE_DOWN);
constexpr int BLOCK_STATE_LINE_VERTICAL = (BLOCK_STATE_LINE_LEFT | BLOCK_STATE_LINE_RIGHT);

//______________
//              \
//				|
//______________/
//
constexpr int BLOCK_STATE_LINE_END_UP = (BLOCK_STATE_LINE_VERTICAL | BLOCK_STATE_LINE_UP);
constexpr int BLOCK_STATE_LINE_END_LEFT = (BLOCK_STATE_LINE_HORIZONTAL | BLOCK_STATE_LINE_LEFT);
constexpr int BLOCK_STATE_LINE_END_DOWN = (BLOCK_STATE_LINE_VERTICAL | BLOCK_STATE_LINE_DOWN);
constexpr int BLOCK_STATE_LINE_END_RIGHT = (BLOCK_STATE_LINE_HORIZONTAL | BLOCK_STATE_LINE_RIGHT);

struct BlockStruct {

	short block_id;
	int block_metadata;
	char block_corner;

	short wall_id[4];
	char wall_corner[4];

	BlockStruct() :
		block_corner(BLOCK_STATE_FULL),
		wall_id{ 0,0,0,0 }
	{}
	BlockStruct(int block_id) :
		block_id(block_id),
		block_metadata(0),
		block_corner(BLOCK_STATE_FULL),
		wall_id{ 0,0,0,0 }
	{}
	inline int wallID() const { return wall_id[0]; }
	inline bool isAir() const { return block_id == 0; }

	//block is either full air or shared
	inline bool isWallFree() const
	{
		return wall_corner[0] != BLOCK_STATE_FULL || wall_id[0] == 0;
	}
	inline void setWall(int id)
	{
		for (int i = 0; i < 4; ++i)
		{
			wall_id[i] = id;
			wall_corner[i] = BLOCK_STATE_FULL;
		}
	}

	//block is either full block or full air
	inline bool isWallOccupied() const
	{
		return (wall_corner[0] == BLOCK_STATE_FULL
			&& wall_corner[1] == BLOCK_STATE_FULL
			&& wall_corner[2] == BLOCK_STATE_FULL
			&& wall_corner[3] == BLOCK_STATE_FULL)
			|| (wall_id[0] == 0
				&& wall_id[1] == 0
				&& wall_id[2] == 0
				&& wall_id[3] == 0);
	}
};

class World;

class Block
{
private:
	const int m_id;

protected:
	//offset in block texture atlas
	half_int m_texture_pos;
	//array[BLOCK_STATE]=TEXTURE_OFFSET
	const half_int* m_corner_translate_array;

	//if yes the texturepos will be added based on location of block in chunk
	bool m_has_big_texture;

	const Phys::Polygon* m_bounds;
	int m_bounds_size;


	//how much light will be consumed by this block 
	//32	-> consumes all light
	//1		-> consumes one light level
	//0		-> consumes no light (you probably dont want that)
	uint8_t m_opacity;

	//0 if not radiating any light
	uint8_t m_light_src;

	int m_block_connect_group;
	bool isInGroup(World* w, int x, int y, int group) const;
	void setNoBounds();

public:
	Block(int id);
	Block(const Block& c) = delete;
	void operator=(Block const&) = delete;
	virtual ~Block();
	inline int getID() const { return m_id; };
	inline int getConnectGroup() const { return m_block_connect_group; }
	inline uint8_t getLightSrcVal() const
	{
		return m_light_src;
	}
	inline uint8_t getOpacity()const {return m_opacity;}

	virtual const Phys::Polygon& getBounds(int x, int y, const BlockStruct b) const;
	bool hasBounds() const;

	inline bool isInConnectGroup(int groups) const { return (groups & m_block_connect_group) != 0; }//they have group in common 

	//returns -1 if not render
	virtual int getTextureOffset(int x, int y, const BlockStruct&) const;
	virtual int getCornerOffset(int x, int y, const BlockStruct&) const;

	//returns true if this block was changed as well
	virtual bool onNeighbourBlockChange(World* world, int x, int y) const;

	virtual void onBlockPlaced(World* w, int x, int y, BlockStruct& b) const {}

#ifdef ND_DEBUG
	inline virtual std::string toString() const { return "UNDEFINED_BLOCK"; }

#define TO_STRING(x)\
	inline std::string toString() const override {return #x;}
#else
#define TO_STRING(X)
#endif
};

class Wall
{
private:
	const int m_id;
protected:
	//offset in block texture atlas
	half_int m_texture_pos;
	//array[BLOCK_STATE]=TEXTURE_OFFSET
	const half_int* m_corner_translate_array;
	bool m_transparent;
public:
	Wall(int id);
	Wall(const Wall& c) = delete;
	void operator=(Wall const&) = delete;
	virtual ~Wall();
	inline int getID() const { return m_id; };
	inline bool isTransparent() const { return m_transparent; }

	//returns -1 if not render
	virtual int getTextureOffset(int wx, int wy, const BlockStruct&) const;
	virtual int getCornerOffset(int wx, int wy, const BlockStruct&) const;

	//returns true if this block was changed as well
	virtual void onNeighbourWallChange(World* world, int x, int y) const;

#ifdef ND_DEBUG
	inline virtual std::string toString() const { return "UNDEFINED_WALL"; }

#define TO_STRING(x)\
	inline std::string toString() const override {return #x;}
#else
#define TO_STRING(X)
#endif
};