#include "ndpch.h"
#include "basic_blocks.h"

//AIR=======================================
BlockAir::BlockAir()
	:Block(BLOCK_AIR)
{
	m_opacity = 0.05f;
	m_texture_offset = -1;
}

//STONE=======================================

BlockStone::BlockStone()
	:Block(BLOCK_STONE)
{
	m_texture_offset = 3;
}

//DIRT========================================
BlockDirt::BlockDirt()
	:Block(BLOCK_DIRT)
{
	m_texture_offset = 2;

}



//GOLD========================================
BlockGold::BlockGold()
	:Block(BLOCK_GOLD)
{
	m_texture_offset =1;

}

//ADAMANTITE==================================

BlockAdamantite::BlockAdamantite()
	:Block(BLOCK_ADAMANTITE)
{
	m_texture_offset = 0;

}
