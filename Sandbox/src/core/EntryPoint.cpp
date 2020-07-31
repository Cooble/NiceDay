#include "Sandbox.h"
#include "world/WorldIO.h"
#include "core/NBT.h"

std::string getName()
{
	static int out = 0;
	return std::to_string(out++);
}
std::string getRandomStringName()
{
	char buff[50];
	auto size = std::rand() % 48 + 1;
	for (int i = 0; i < size; ++i)
	{
		buff[i] = std::rand() % 100 + 65;
	}
	buff[size] = 0;
	

	//return std::string(buff,size);
	return "kbj";
}




float testUnorderedMap(int size)
{

	constexpr int maxSize = 100000;
	typedef uint64_t lookType;
	std::unordered_map<lookType, lookType> map;

	std::vector<lookType> keys;
	keys.resize(size);
	std::vector<lookType> keysRandomIndexes;
	keysRandomIndexes.resize(size);
	
	for (int i = 0; i < size; ++i)
	{
		keys[i] = std::rand();
		map[keys[i]] = std::rand();
		keysRandomIndexes[i] = std::rand() % size;
	}
	std::vector<lookType> sim;
	sim.resize(size);
	for (int i = 0; i < size; ++i)
	{
		sim[i] = keys[keysRandomIndexes[i]];
	}

	{
		TimerStaper t("unordered performance");
		constexpr int batches = 10;
		for (int i = 0; i < batches; ++i)
		{
			for (int i = 0; i < size; ++i)
			{
				uint64_t ii = map[sim[i]];

			}
		}
		
		return (float)t.getUS() / size / batches;
	}
	
}

void testUnorderedMap()
{
	auto t = { 10,100,500,1000,4000,10000,20000,50000,100000 };

	for (auto t1 : t)
	{
		int batches = 100;
		float time = 0;
		for (int i = 0; i < batches; ++i)
		  time += testUnorderedMap(t1);
		
		ND_INFO("[{}] -> {}", t1, time/batches);
	}
	
}
#include <nlohmann/json.hpp>
#ifndef ND_TEST
int main()
{
	
	Log::init();
	ND_PROFILE_BEGIN_SESSION("start", "start.json");
	ResourceMan::init();
	Sandbox game;

	ND_PROFILE_END_SESSION();
	game.start();

	//std::cin.get();

	Log::flush();
	return 0;
}
#endif
