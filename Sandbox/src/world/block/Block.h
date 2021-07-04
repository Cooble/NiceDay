#pragma once
#include "ndpch.h"
#include "core/physShapes.h"
#include "world/entity/EntityManager.h"
#include "world/entity/entity_datas.h"
#include "inventory/Item.h"
class ItemWall;
class BlockRegistry;
class BlockAccess;
class BlockTextureAtlas;
//STATES=========================================================
//is air counterclockwise start from up(=lsb) (up left down right)
constexpr int BLOCK_STATE_FULL = 0;
//		  ______________
//		 <              >
//		 >			       <
//		 <\/\/\/\/\/\/\/>
//
constexpr int BLOCK_STATE_LINE_UP = BIT(0);
constexpr int BLOCK_STATE_LINE_DOWN = BIT(2);
constexpr int BLOCK_STATE_LINE_LEFT = BIT(1);
constexpr int BLOCK_STATE_LINE_RIGHT = BIT(3);
constexpr int BLOCK_STATE_VARIABLE = BIT(4);
constexpr int BLOCK_STATE_CRACKED = BIT(5);
constexpr int BLOCK_STATE_HAMMER = BIT(6);

//all bits except for cracked
constexpr int BLOCK_STATE_PURE_MASK = BLOCK_STATE_LINE_UP | BLOCK_STATE_LINE_DOWN | BLOCK_STATE_LINE_LEFT |
	BLOCK_STATE_LINE_RIGHT;


//     ____
//    |      \     
//    |		    \
//    |__________|
//
constexpr int BLOCK_STATE_CORNER_UP_LEFT = BLOCK_STATE_LINE_UP | BLOCK_STATE_LINE_LEFT;
constexpr int BLOCK_STATE_CORNER_UP_RIGHT = BLOCK_STATE_LINE_UP | BLOCK_STATE_LINE_RIGHT;
constexpr int BLOCK_STATE_CORNER_DOWN_LEFT = BLOCK_STATE_LINE_DOWN | BLOCK_STATE_LINE_LEFT;
constexpr int BLOCK_STATE_CORNER_DOWN_RIGHT = BLOCK_STATE_LINE_DOWN | BLOCK_STATE_LINE_RIGHT;

constexpr int BLOCK_STATE_BIT = BLOCK_STATE_CORNER_UP_LEFT | BLOCK_STATE_CORNER_DOWN_RIGHT;
//		  ______________
//		 <              >
//		 >			       <
//		 <______________>
//
constexpr int BLOCK_STATE_LINE_HORIZONTAL = BLOCK_STATE_LINE_UP | BLOCK_STATE_LINE_DOWN;
constexpr int BLOCK_STATE_LINE_VERTICAL = BLOCK_STATE_LINE_LEFT | BLOCK_STATE_LINE_RIGHT;

//______________
//              \
//				    |
//______________/
//
constexpr int BLOCK_STATE_LINE_END_UP = BLOCK_STATE_LINE_VERTICAL | BLOCK_STATE_LINE_UP;
constexpr int BLOCK_STATE_LINE_END_LEFT = BLOCK_STATE_LINE_HORIZONTAL | BLOCK_STATE_LINE_LEFT;
constexpr int BLOCK_STATE_LINE_END_DOWN = BLOCK_STATE_LINE_VERTICAL | BLOCK_STATE_LINE_DOWN;
constexpr int BLOCK_STATE_LINE_END_RIGHT = BLOCK_STATE_LINE_HORIZONTAL | BLOCK_STATE_LINE_RIGHT;

typedef int BlockID;

struct BlockStruct
{
	short block_id;
	int block_metadata;
	char block_corner;

	short wall_id[4];
	char wall_corner[4];

	BlockStruct() :
		block_corner(BLOCK_STATE_FULL),
		wall_id{0, 0, 0, 0} {}

	BlockStruct(int block_id) :
		block_id(block_id),
		block_metadata(0),
		block_corner(BLOCK_STATE_FULL),
		wall_id{0, 0, 0, 0} {}

	int wallID() const { return wall_id[0]; }
	bool isAir() const { return block_id == 0; }

	//block is either full air or shared
	bool isWallFree() const { return wall_corner[0] != BLOCK_STATE_FULL || wall_id[0] == 0; }

	void setWall(int id)
	{
		for (int i = 0; i < 4; ++i)
		{
			wall_id[i] = id;
			wall_corner[i] = BLOCK_STATE_FULL;
		}
	}

	//block is either full block or full air
	// i.e is not partially filled
	bool isWallFullyOccupied() const
	{
		return (*((int*)wall_corner) == (BLOCK_STATE_FULL << 24) | (BLOCK_STATE_FULL << 16) | (BLOCK_STATE_FULL << 8) | (BLOCK_STATE_FULL << 0))
		||(*((int*)wall_corner) == 0);
	}
};

class World;
class WorldEntity;

constexpr uint8_t OPACITY_AIR = 1;
constexpr uint8_t OPACITY_SOLID = 3;

constexpr int BLOCK_FLAG_HAS_ITEM_VERSION = 0;

// automatically get texture uv offset by adding meta to x coordinate of texture uv
constexpr int BLOCK_FLAG_HAS_METATEXTURES_IN_ROW = 1;
//block needs wall behind it (e.g Painting)
constexpr int BLOCK_FLAG_NEEDS_WALL = 2;
//block cannot float in the air
constexpr int BLOCK_FLAG_CANNOT_FLOAT = 3;

constexpr int BLOCK_FLAG_HAS_BIG_TEXTURE = 4;

//on block can be placed candle or something
constexpr int BLOCK_FLAG_SOLID = 5;

//can be replaced by another block for example flowers
constexpr int BLOCK_FLAG_REPLACEABLE = 6;

//can be replaced by another block for example flowers
constexpr int BLOCK_FLAG_TRANSPARENT = 7;


class ItemBlock;

class Block
{
	friend BlockRegistry;
private:
	int m_id;
	const std::string m_string_id;
protected:
	int m_flags = 0;
	float m_hardness = 1;
	int m_tier = 1;
	int m_required = 0;

	/**
	 * when one block has for example more flower species this number tells us how many
	 * used by itemblock
	 */
	int m_max_metadata = 0;

	//offset in block texture atlas
	half_int m_texture_pos;
	//array[BLOCK_STATE]=TEXTURE_OFFSET
	const half_int* m_corner_translate_array;

	//if yes the texturepos will be added based on location of block in chunk
	//bool m_has_big_texture;

	EntityType m_tile_entity;

	const ndPhys::Polygon* m_collision_box;
	int m_collision_box_size;


	//how much light will be consumed by this block 
	//32	-> consumes all light
	//1		-> consumes one light level
	//0		-> consumes no light (you probably dont want that)
	uint8_t m_opacity;

	//0 if not radiating any light
	uint8_t m_light_src;

	int m_block_connect_group;
	bool isInGroup(BlockAccess& w, int x, int y, int group) const;
	bool isInGroup(BlockID blockID, int group) const;

	void setNoCollisionBox()
	{
		m_collision_box = nullptr;
		m_collision_box_size = 0;
	}

	void setFlag(int flag, bool value = true)
	{
		if (value)
			m_flags |= 1 << flag;
		else
			m_flags &= ~(1 << flag);
	}

public:
	Block(std::string id);
	Block(const Block& c) = delete;
	void operator=(const Block&) = delete;
	virtual ~Block() = default;

	// the bigger the number the more time it will take to dig it
	// 0 means insta dig
	float getHardness() const { return m_hardness; }

	// lowest necessary tool tier to be able to dig it
	int getTier() const { return m_tier; }

	int getID() const { return m_id; };
	int getConnectGroup() const { return m_block_connect_group; }
	int getMaxMetadata() const { return m_max_metadata; }
	uint8_t getLightSrcVal() const { return m_light_src; }
	uint8_t getOpacity() const { return m_opacity; }
	bool hasTileEntity() const { return m_tile_entity != ENTITY_TYPE_NONE; }
	virtual ndPhys::Vecti getTileEntityCoords(int x, int y, const BlockStruct& b) const;
	EntityType getTileEntity() const { return m_tile_entity; }

	virtual const ndPhys::Polygon& getCollisionBox(int x, int y, const BlockStruct& b) const;
	bool hasCollisionBox() const;

	bool isInConnectGroup(int groups) const { return (groups & m_block_connect_group) != 0; }
	//they have group in common 

	//basic flags
	constexpr bool hasItemVersion() const { return m_flags & 1 << BLOCK_FLAG_HAS_ITEM_VERSION; }
	constexpr bool hasMetaTexturesInRow() const { return m_flags & 1 << BLOCK_FLAG_HAS_METATEXTURES_IN_ROW; }
	constexpr bool needsWall() const { return m_flags & 1 << BLOCK_FLAG_NEEDS_WALL; }
	constexpr bool cannotFloat() const { return m_flags & 1 << BLOCK_FLAG_CANNOT_FLOAT; }
	constexpr bool hasBigTexture() const { return m_flags & 1 << BLOCK_FLAG_HAS_BIG_TEXTURE; }
	constexpr bool isSolid() const { return m_flags & 1 << BLOCK_FLAG_SOLID; }
	constexpr bool isReplaceable() const { return m_flags & 1 << BLOCK_FLAG_REPLACEABLE; }
	constexpr bool hasFlag(int flag) const { return m_flags & 1 << flag; }


	virtual void onTextureLoaded(const BlockTextureAtlas& atlas);

	//returns -1 if not render
	virtual int getTextureOffset(int x, int y, const BlockStruct& b) const;
	virtual int getCornerOffset(int x, int y, const BlockStruct& b) const;

	//returns true if this block was changed as well
	virtual bool onNeighborBlockChange(BlockAccess& world, int x, int y) const;

	virtual void onBlockPlaced(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const;
	virtual void onBlockDestroyed(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const;


	virtual void onBlockClicked(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const;

	virtual bool canBePlaced(World& w, int x, int y) const;


	virtual const ItemBlock& getItemFromBlock() const;
	virtual std::string getItemIDFromBlock() const;

	virtual ItemStack* createItemStackFromBlock(const BlockStruct& b) const;
	const std::string& getStringID() const { return m_string_id; }
};

class MultiBlock : public Block
{
	friend BlockRegistry;
protected:
	ndPhys::Rectanglei m_build_dimensions;
	int m_width;
	int m_height;

	template <int Width, int Height>
	void generateCollisionBoxFromDimensions()
	{
		static ndPhys::Polygon p = ndPhys::toPolygon(ndPhys::Rectangle::createFromDimensions(0, 0, Width, Height));
		m_collision_box = &p;
	}

public:
	MultiBlock(std::string id);
	~MultiBlock() override = default;
	MultiBlock(const Block& c) = delete;
	void operator=(const MultiBlock&) = delete;

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }

	ndPhys::Vecti getTileEntityCoords(int x, int y, const BlockStruct& b) const override;

	//what dimensions need to be placed
	virtual ndPhys::Rectanglei getBuildDimensions() const { return m_build_dimensions; }

	int getTextureOffset(int x, int y, const BlockStruct& b) const override;

	bool onNeighborBlockChange(BlockAccess& world, int x, int y) const override;

	void onBlockPlaced(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const override;

	void onBlockDestroyed(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const override;

	bool canBePlaced(World& w, int x, int y) const override;
};

class Wall
{
	friend BlockRegistry;
private:
	int m_id;
	const std::string m_string_id;
	int m_flags=0;
protected:
	//offset in block texture atlas
	half_int m_texture_pos;
	//array[BLOCK_STATE]=TEXTURE_OFFSET
	const half_int* m_corner_translate_array;
public:
	Wall(std::string id);
	Wall(const Wall& c) = delete;
	void operator=(const Wall&) = delete;
	virtual ~Wall();
	virtual void onTextureLoaded(const BlockTextureAtlas& atlas);
	int getID() const { return m_id; };

	//flags

	void setFlag(int flag, bool value = true)
	{
		if (value)
			m_flags |= 1 << flag;
		else
			m_flags &= ~(1 << flag);
	}
	constexpr bool hasItemVersion() const { return m_flags & 1 << BLOCK_FLAG_HAS_ITEM_VERSION; }
	constexpr bool isReplaceable() const { return m_flags & 1 << BLOCK_FLAG_REPLACEABLE; }
	constexpr bool isTransparent() const { return m_flags & 1 << BLOCK_FLAG_TRANSPARENT; }

	//returns -1 if not render
	virtual int getTextureOffset(int wx, int wy, const BlockStruct&) const;
	virtual int getCornerOffset(int wx, int wy, const BlockStruct&) const;

	//returns true if this block was changed as well
	virtual void onNeighbourWallChange(BlockAccess& world, int x, int y) const;
	virtual bool canBePlaced(World& w, int x, int y) const;
	virtual const ItemWall& getItemFromWall() const;
	virtual ItemStack* createItemStackFromWall(const BlockStruct& b) const;


	virtual std::string getItemIDFromWall() const;
	const std::string& getStringID() const { return m_string_id; }
};
