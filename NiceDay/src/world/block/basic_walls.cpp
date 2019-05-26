#include "ndpch.h"
#include "basic_walls.h"
#include "block_datas.h"

//AIR=============================
WallAir::WallAir()
	:Wall(WALL_AIR)
{
	
}

int WallAir::getTextureOffset(int wx, int wy, const BlockStruct&) const
{
	return -1;
}

int WallAir::getCornerOffset(int wx, int wy, const BlockStruct&) const
{
	return 0;
}




//DIRT============================
WallDirt::WallDirt()
	:Wall(WALL_DIRT)
{
	m_corner_translate_array = WALL_CORNERS_DIRT;
	m_texture_pos = half_int(0,1);
}

WallStone::WallStone()
	:Wall(WALL_STONE)
{
	m_corner_translate_array = WALL_CORNERS_DIRT;
	m_texture_pos = half_int(1, 1);
}
