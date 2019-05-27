#pragma once
#define BLOCK_STATE_FULL				0
//		  ______________
//		 <              >
//		 >			    <
//		 <\/\/\/\/\/\/\/>
//
#define BLOCK_STATE_LINE_UP				1
#define BLOCK_STATE_LINE_DOWN			2
#define BLOCK_STATE_LINE_LEFT			3
#define BLOCK_STATE_LINE_RIGHT			4

//     ____
//    |      \     
//    |		    \
//    |__________|
//
#define BLOCK_STATE_CORNER_UP_LEFT		5
#define BLOCK_STATE_CORNER_UP_RIGHT		6
#define BLOCK_STATE_CORNER_DOWN_LEFT	7
#define BLOCK_STATE_CORNER_DOWN_RIGHT	8

#define BLOCK_STATE_BIT					9
//		  ______________
//		 <              >
//		 >			    <
//		 <______________>
//
#define BLOCK_STATE_LINE_HORIZONTAL		10
#define BLOCK_STATE_LINE_VERTICAL		11

//______________
//              \
//				|
//______________/
//
#define BLOCK_STATE_LINE_END_UP			12
#define BLOCK_STATE_LINE_END_LEFT		13
#define BLOCK_STATE_LINE_END_DOWN		14
#define BLOCK_STATE_LINE_END_RIGHT		15

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

	//how much light will be consumed by this block 
	//1		-> consumes all light
	//0.5	-> consumes half of light passing through
	//0		-> consumes no light
	float m_opacity;

	int m_block_connect_group;
	bool isInGroup(World* w, int x, int y, int group) const;
public:
	Block(int id);
	Block(const Block& c) = delete;
	void operator=(Block const&) = delete;
	virtual ~Block();
	inline int getID() const { return m_id; };
	inline int getConnectGroup() const { return m_block_connect_group; }
	virtual float getOpacity(const BlockStruct&) const;

	inline bool isInConnectGroup(int groups) const { return (groups & m_block_connect_group) != 0; }//they have group in common 

	//returns -1 if not render
	virtual int getTextureOffset(int x, int y, const BlockStruct&) const;
	virtual int getCornerOffset(int x, int y, const BlockStruct&) const;

	//returns true if this block was changed as well
	virtual bool onNeighbourBlockChange(World* world, int x, int y) const;

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

	bool m_opaque;
public:
	Wall(int id);
	Wall(const Wall& c) = delete;
	void operator=(Wall const&) = delete;
	virtual ~Wall();
	inline int getID() const { return m_id; };
	virtual bool isOpaque(const BlockStruct&) const;

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