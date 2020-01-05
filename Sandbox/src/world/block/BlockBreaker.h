#pragma once

class World;

class BlockBreaker
{
private:
	int m_time;
	int m_x, m_y;
public:
	BlockBreaker();

	void beginBreak(World& w,int x, int y);
	
	bool update(World& w);
};
