#pragma once
#include "ndpch.h"
#include "Game.h"

int main()
{
	Log::Init();
	ND_WARN("Initialized Log!");
	
	Game game;
	game.init();
	game.start();
}
