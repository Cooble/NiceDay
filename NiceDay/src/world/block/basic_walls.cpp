#include "ndpch.h"
#include "basic_walls.h"
#include "block_datas.h"

//AIR=============================
WallAir::WallAir()
	: Wall(WALL_AIR)
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
	: Wall(WALL_DIRT)
{
	m_corner_translate_array = WALL_CORNERS_DIRT;
	m_texture_pos = half_int(0, 1);
}

WallStone::WallStone()
	: Wall(WALL_STONE)
{
	m_corner_translate_array = WALL_CORNERS_DIRT;
	m_texture_pos = half_int(1, 1);
}

WallGlass::WallGlass()
	: Wall(WALL_GLASS)
{
	m_transparent = true;
	m_corner_translate_array = WALL_CORNERS_GLASS;
	m_texture_pos = half_int(3, 1);
}

int WallGlass::getCornerOffset(int wx, int wy, const BlockStruct& b) const
{
	ASSERT(m_corner_translate_array, " m_corner_translate_array  cannot be null");
	half_int i = m_corner_translate_array[(int)b.wall_corner[(wy & 1) * 2 + (wx & 1)]];
	if (i.i == 0)

		return 0;

	return i;
}
