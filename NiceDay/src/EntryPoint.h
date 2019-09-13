#pragma once
#include "ndpch.h"
#include "App.h"
#include "Sandbox.h"

int main()
{
	Sandbox game;	
	game.start();

	std::cin.get();

	return 0;
}
