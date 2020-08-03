#include "Sandbox.h"
#include "world/WorldIO.h"
#ifndef ND_TEST
int main()
{
	
	Log::init();
	ND_PROFILE_BEGIN_SESSION("start", "start.json");
	Sandbox game;

	ND_PROFILE_END_SESSION();
	game.start();

	//std::cin.get();

	Log::flush();
	return 0;
}
#endif
