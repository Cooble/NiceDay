#include "BlockBreaker.h"
#include "world/World.h"

BlockBreaker::BlockBreaker() {}

void BlockBreaker::beginBreak(World& w, int x, int y)
{
	m_x = x;
	m_y = y;
	auto blok = w.getBlock(x, y);
	ASSERT(blok, "cannot break blok");
	m_time = BlockRegistry::get().getBlock(blok->block_id).getHardness() * 60;
}

bool BlockBreaker::update(World& w)
{
	if (m_time-- > 0) { }
	if (m_time == 0) { }
	return true;
}
