#pragma once
#define BLOCK_STATE_FULL				0
#define BLOCK_STATE_BIT					56
#define BLOCK_STATE_BOLD_LINE_UP		48
#define BLOCK_STATE_BOLD_LINE_DOWN		40
#define BLOCK_STATE_BOLD_LINE_LEFT		32
#define BLOCK_STATE_BOLD_LINE_RIGHT		24
#define BLOCK_STATE_THIN_LINE_HORIZONTAL 53
#define BLOCK_STATE_THIN_LINE_VERTICAL	52

#define BLOCK_STATE_THIN_LINE_END_UP	60
#define BLOCK_STATE_THIN_LINE_END_LEFT	61
#define BLOCK_STATE_THIN_LINE_END_DOWN	62
#define BLOCK_STATE_THIN_LINE_END_RIGHT 63

#define BLOCK_STATE_CORNER_UP_LEFT		44
#define BLOCK_STATE_CORNER_UP_RIGHT		45
#define BLOCK_STATE_CORNER_DOWN_LEFT	36
#define BLOCK_STATE_CORNER_DOWN_RIGHT	37

#define BLOCK_GROUP_AIR_BIT			0
#define BLOCK_GROUP_DIRT_BIT		1
#define BLOCK_GROUP_PLATFORM_BIT	2


const int BLOCK_AIR			= 0;
const int BLOCK_STONE		= 1;
const int BLOCK_DIRT		= 2;
const int BLOCK_GOLD		= 3;
const int BLOCK_ADAMANTITE	= 4;
const int BLOCK_PLATFORM	= 5;

struct BlockStruct {
	int id;
	int metadata;
	int corner;
	BlockStruct():corner(BLOCK_STATE_FULL){}
	BlockStruct(int idd) :id(idd), metadata(0), corner(BLOCK_STATE_FULL)
	{}

	inline bool isAir() const { return id == BLOCK_AIR; }
};

struct half_int
{
	union
	{
		uint32_t i;
		struct
		{
			uint16_t x;//lsb
			uint16_t y;//msb
		};
	};
	half_int(int in):i(in){}
	half_int(int xx,int yy):x((short)xx),y((short)yy){}
};

class World;

class Block
{
private:
	const int m_id;

protected:
	//offset in block texture atlas
	half_int m_texture_pos;

	//if yes the texturepos will be added based on location of block in chunk
	bool m_has_big_texture;

	//how much light will be consumed by this block 
	//1		-> consumes all light
	//0.5	-> consumes half of light passing through
	//0		-> consumes no light
	float m_opacity;

	int m_block_connect_group;
	static bool isInGroup(World* w, int x, int y, int group);
public:
	Block(int id);
	Block(const Block& c) = delete;
	void operator=(Block const&) = delete;
	virtual ~Block();
	inline int getID() const { return m_id; };
	inline int getConnectGroup() const { return m_block_connect_group; }
	virtual float getOpacity(const BlockStruct&) const;
	
	inline bool isInConnectGroup(int groups) const { return (groups & m_block_connect_group) == groups; }
	
	//returns -1 if not render
	virtual int getTextureOffset(int x,int y,const BlockStruct&) const;
	virtual int getCornerOffset(int x,int y,const BlockStruct&) const;

	//returns true if this block was changed as well
	virtual bool onNeighbourBlockChange(World* world, int x, int y) const;

#ifdef ND_DEBUG
	inline virtual std::string toString() const { return "UNDEFINED_BLOCK"; }
	
	#define BLOCK_TO_STRING(x)\
	inline std::string toString() const override {return #x;}
#else
	#define BLOCK_TO_STRING(X)
#endif
};

