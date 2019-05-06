#pragma once
#include "ndpch.h"
#include "Game.h"

int main()
{
	Log::Init();	
	Game game;
	game.init();
	game.start();
	std::cin.get();

	return 0;
}
