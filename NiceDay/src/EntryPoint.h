#pragma once
#include "ndpch.h"
#include "Game.h"


int main()
{
	/*NDUtil::FifoList<int> list(2);

	list.push(1);
	list.push(2);
	list.push(3);
	list.popMode();
	int x = list.pop();
	int y = list.pop();
	int z = list.pop();

	list.push(5);
	list.push(6);
	list.push(7);
	list.popMode();
	while (!list.empty())
		int g = list.pop();
	*/





	
	Log::Init();	
	Game game;
	game.init();
	game.start();
	std::cin.get();

	return 0;
}
