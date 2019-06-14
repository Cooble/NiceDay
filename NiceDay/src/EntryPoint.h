#pragma once
#include "ndpch.h"
#include "Game.h"

#include <bitset>
#include <climits>

/*template<typename T>
void show_binrep(const T& a)
{
	const char* beg = reinterpret_cast<const char*>(&a);
	const char* end = beg + sizeof(a);
	while (beg != end)
		std::cout << std::bitset<CHAR_BIT>(*beg++) << ' ';
	std::cout << '\n';
}*/

void show_binrep(const char& a)
{
	ND_INFO((int)a);
	const char* beg = reinterpret_cast<const char*>(&a);
	const char* end = beg + sizeof(a);
	while (beg != end)
		std::cout << std::bitset<CHAR_BIT>(*beg++) << ' ';
	std::cout << '\n';
}

int main()
{
	Log::Init();

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





	
	Game game;
	game.init();
	game.start();
	std::cin.get();

	return 0;
}
