#pragma once
#include "ndpch.h"
#include "core/physShapes.h"
//=======================GROUPs=======================

#define BLOCK_GROUP_AIR_BIT			0
#define BLOCK_GROUP_DIRT_BIT		1
#define BLOCK_GROUP_PLATFORM_BIT	2
#define BLOCK_GROUP_ORE_BIT			3

//=======================BLOCK IDs=======================

constexpr int BLOCK_AIR =			0;
constexpr int BLOCK_STONE =			1;
constexpr int BLOCK_DIRT =			2;
constexpr int BLOCK_GOLD =			3;
constexpr int BLOCK_ADAMANTITE =	4;
constexpr int BLOCK_PLATFORM =		5;
constexpr int BLOCK_GRASS =			6;
constexpr int BLOCK_GLASS =			7;
constexpr int BLOCK_TORCH =			8;
constexpr int BLOCK_DOOR_CLOSE =	9;
constexpr int BLOCK_DOOR_OPEN =		10;
constexpr int BLOCK_PAINTING =		11;
constexpr int BLOCK_TREE =			12;
constexpr int BLOCK_TREE_SAPLING =	13;
constexpr int BLOCK_FLOWER =		14;
constexpr int BLOCK_GRASS_PLANT =	15;

constexpr int BLOCK_SNOW =			16;
constexpr int BLOCK_SNOW_BRICK =	17;
constexpr int BLOCK_ICE =			18;
constexpr int BLOCK_PUMPKIN =		19;
constexpr int BLOCK_CHEST =			20;

constexpr int BLOCK_RADIO =			21;

//=======================WALL IDs=======================

constexpr int WALL_AIR =			0;
constexpr int WALL_DIRT =			1;
constexpr int WALL_STONE =			2;
constexpr int WALL_GLASS =			3;

//=======================BLOCKSTATE Ids=======================
//00 BLOCK_STATE_FULL
//01 BLOCK_STATE_LINE_UP
//02 BLOCK_STATE_LINE_LEFT
//03 BLOCK_STATE_CORNER_UP_LEFT
//04 BLOCK_STATE_LINE_DOWN
//05 BLOCK_STATE_LINE_HORIZONTAL
//06 BLOCK_STATE_CORNER_DOWN_LEFT
//07 BLOCK_STATE_LINE_END_LEFT
//08 BLOCK_STATE_LINE_RIGHT
//09 BLOCK_STATE_CORNER_UP_RIGHT
//10 BLOCK_STATE_LINE_VERTICAL
//11 BLOCK_STATE_LINE_END_UP
//12 BLOCK_STATE_CORNER_DOWN_RIGHT
//13 BLOCK_STATE_LINE_END_RIGHT
//14 BLOCK_STATE_LINE_END_DOWN
//15 BLOCK_STATE_BIT

//=======================CORNER MAPPING=======================

const half_int BLOCK_CORNERS_DIRT[16] = {
	half_int(0,0),//00 BLOCK_STATE_FULL
	half_int(0,6),//01 BLOCK_STATE_LINE_UP
	half_int(0,4),//02 BLOCK_STATE_LINE_LEFT
	half_int(4,5),//03 BLOCK_STATE_CORNER_UP_LEFT
	half_int(0,5),//04 BLOCK_STATE_LINE_DOWN
	half_int(5,6),//05 BLOCK_STATE_LINE_HORIZONTAL
	half_int(6,5),//06 BLOCK_STATE_CORNER_DOWN_LEFT
	half_int(5,7),//07 BLOCK_STATE_LINE_END_LEFT
	half_int(0,3),//08 BLOCK_STATE_LINE_RIGHT
	half_int(5,5),//09 BLOCK_STATE_CORNER_UP_RIGHT
	half_int(4,6),//10 BLOCK_STATE_LINE_VERTICAL
	half_int(4,7),//11 BLOCK_STATE_LINE_END_UP
	half_int(7,5),//12 BLOCK_STATE_CORNER_DOWN_RIGHT
	half_int(7,7),//13 BLOCK_STATE_LINE_END_RIGHT
	half_int(6,7),//14 BLOCK_STATE_LINE_END_DOWN
	half_int(0,7),//15 BLOCK_STATE_BIT
};
const half_int BLOCK_CORNERS_GLASS[16] = {
	half_int(0,0),//00 BLOCK_STATE_FULL
	half_int(2,6),//01 BLOCK_STATE_LINE_UP
	half_int(2,4),//02 BLOCK_STATE_LINE_LEFT
	half_int(4,2),//03 BLOCK_STATE_CORNER_UP_LEFT
	half_int(2,5),//04 BLOCK_STATE_LINE_DOWN
	half_int(5,3),//05 BLOCK_STATE_LINE_HORIZONTAL
	half_int(6,2),//06 BLOCK_STATE_CORNER_DOWN_LEFT
	half_int(5,4),//07 BLOCK_STATE_LINE_END_LEFT
	half_int(2,3),//08 BLOCK_STATE_LINE_RIGHT
	half_int(5,2),//09 BLOCK_STATE_CORNER_UP_RIGHT
	half_int(4,3),//10 BLOCK_STATE_LINE_VERTICAL
	half_int(4,4),//11 BLOCK_STATE_LINE_END_UP
	half_int(7,2),//12 BLOCK_STATE_CORNER_DOWN_RIGHT
	half_int(7,4),//13 BLOCK_STATE_LINE_END_RIGHT
	half_int(6,4),//14 BLOCK_STATE_LINE_END_DOWN
	half_int(2,7),//15 BLOCK_STATE_BIT
};
const half_int WALL_CORNERS_DIRT[16] = {
	half_int(0,0),//00 BLOCK_STATE_FULL
	half_int(3,2),//01 BLOCK_STATE_LINE_UP
	half_int(2,1),//02 BLOCK_STATE_LINE_LEFT
	half_int(2,2),//03 BLOCK_STATE_CORNER_UP_LEFT
	half_int(3,0),//04 BLOCK_STATE_LINE_DOWN
	half_int(0,0),//05 BLOCK_STATE_LINE_HORIZONTAL INVALID
	half_int(2,0),//06 BLOCK_STATE_CORNER_DOWN_LEFT
	half_int(0,0),//07 BLOCK_STATE_LINE_END_LEFT INVALID
	half_int(4,1),//08 BLOCK_STATE_LINE_RIGHT
	half_int(4,2),//09 BLOCK_STATE_CORNER_UP_RIGHT
	half_int(0,0),//10 BLOCK_STATE_LINE_VERTICAL INVALID
	half_int(0,0),//11 BLOCK_STATE_LINE_END_UP INVALID
	half_int(4,0),//12 BLOCK_STATE_CORNER_DOWN_RIGHT
	half_int(0,0),//13 BLOCK_STATE_LINE_END_RIGHT INVALID
	half_int(0,0),//14 BLOCK_STATE_LINE_END_DOWN INVALID
	half_int(0,0),//15 BLOCK_STATE_BIT INVALID
};

const half_int WALL_CORNERS_GLASS[16] = {
	half_int(0+0,0),//00 BLOCK_STATE_FULL
	half_int(6+3,2),//01 BLOCK_STATE_LINE_UP
	half_int(6+2,1),//02 BLOCK_STATE_LINE_LEFT
	half_int(6+2,2),//03 BLOCK_STATE_CORNER_UP_LEFT
	half_int(6+3,0),//04 BLOCK_STATE_LINE_DOWN
	half_int(6+0,0),//05 BLOCK_STATE_LINE_HORIZONTAL INVALID
	half_int(6+2,0),//06 BLOCK_STATE_CORNER_DOWN_LEFT
	half_int(6+0,0),//07 BLOCK_STATE_LINE_END_LEFT INVALID
	half_int(6+4,1),//08 BLOCK_STATE_LINE_RIGHT
	half_int(6+4,2),//09 BLOCK_STATE_CORNER_UP_RIGHT
	half_int(6+0,0),//10 BLOCK_STATE_LINE_VERTICAL INVALID
	half_int(6+0,0),//11 BLOCK_STATE_LINE_END_UP INVALID
	half_int(6+4,0),//12 BLOCK_STATE_CORNER_DOWN_RIGHT
	half_int(6+0,0),//13 BLOCK_STATE_LINE_END_RIGHT INVALID
	half_int(6+0,0),//14 BLOCK_STATE_LINE_END_DOWN INVALID
	half_int(6+0,0),//15 BLOCK_STATE_BIT INVALID
};

const Phys::Polygon BLOCK_BOUNDS_DEFAULT[3] = {
	Phys::toPolygon(Phys::Rectangle::createFromDimensions(0,0,1,1)),//00 BLOCK_STATE_FULL
	Phys::Polygon({{0,0},{1,0},{1,1}}) ,//00 LEFT_CORNER
	Phys::Polygon({{0,0},{1,0},{0,1}}) ,//00 RIGHT_CORNER
};
const Phys::Polygon BLOCK_BOUNDS_DOOR = Phys::toPolygon(Phys::Rectangle::createFromDimensions(0.25,0,0.5,1));
const Phys::Polygon BLOCK_BOUNDS_PLATFORM = Phys::toPolygon(Phys::Rectangle::createFromDimensions(0.75,0,1,0.25));

