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

#define BLOCK_STATE_CORNER_UP_LEFT 44
#define BLOCK_STATE_CORNER_UP_RIGHT 45
#define BLOCK_STATE_CORNER_DOWN_LEFT 36
#define BLOCK_STATE_CORNER_DOWN_RIGHT 37



const int BLOCK_AIR			= 0;
const int BLOCK_STONE		= 1;
const int BLOCK_DIRT		= 2;
const int BLOCK_GOLD		= 3;
const int BLOCK_ADAMANTITE	= 4;

struct BlockStruct {
	int id;
	int metadata;
	int corner;
	BlockStruct() = default;
	BlockStruct(int idd) :id(idd), metadata(0), corner(0)
	{}

	inline bool isAir() const { return id == BLOCK_AIR; }
};

class World;

class Block
{
private:
	const int m_id;

protected:

	//how much light will be consumed by this block 
	//1		-> consumes all light
	//0.5	-> consumes half of light passing through
	//0		-> consumes no light
	float m_opacity;

	//offset in block texture atlas
	int m_texture_offset;
public:
	Block(int id);
	Block(const Block& c) = delete;
	void operator=(Block const&) = delete;
	virtual ~Block();
	inline int getID() const { return m_id; };
	virtual float getOpacity(const BlockStruct&) const;
	//returns -1 if not render
	virtual int getTextureOffset(const BlockStruct&) const;
	virtual int getCornerOffset(const BlockStruct&) const;

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

