#include "ndpch.h"
#include "basic_walls.h"
#include "block_datas.h"

//AIR=============================
WallAir::WallAir()
	: Wall("air")
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

WallGlass::WallGlass()
	: Wall("glass")
{
}

int WallGlass::getCornerOffset(int wx, int wy, const BlockStruct& b) const
{
	ASSERT(m_corner_translate_array, " m_corner_translate_array  cannot be null");
	half_int i = m_corner_translate_array[(int)b.wall_corner[(wy & 1) * 2 + (wx & 1)]];
	return i;
}
