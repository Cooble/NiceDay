#pragma once
#include "ndpch.h"
#include "Game.h"

#include <bitset>
#include <climits>

#include "world/entity/EntityRegistry.h"
#include "world/entity/entities.h"
#include "world/entity/entity_datas.h"
#include "nbt/NBT.h"

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
void createRandomNBT(NBT& nbt,int size = 0);
bool equalsNBT(NBT& one, NBT& two);

int main()
{

	Log::Init();
	auto stream = std::fstream("src/data.example", std::ios::out | std::ios::binary);
	NBT one;
	one.set<std::string>("helpme", std::string("this is a value of happines"));
	createRandomNBT(one);
	one.save(stream);
	stream.close();

	NBT second;
	stream = std::fstream("src/data.example", std::ios::in | std::ios::binary);
	stream.seekg(std::fstream::beg);
	second.load(stream);
	stream.close();

	ND_INFO("nbts equal {}", equalsNBT(one, second));

	one.~NBT();
	second.~NBT();


	/*ND_REGISTER_ENTITY(ENTITY_TYPE_PLAYER, EntityPlayer);

	void* buff = malloc(sizeof(EntityPlayer));
	EntityRegistry::get().createInstance(ENTITY_TYPE_PLAYER, buff);

	auto* karle = (WorldEntity*)buff;
	auto* pPlayer = (EntityPlayer*) buff;

	ND_INFO("ID of player is: {}", pPlayer->getEntityType());
	ND_INFO("ID of player is: {}", EntityRegistry::get().entityTypeToString(karle->getEntityType()));
	ND_INFO("size of player is: {}", EntityRegistry::get().getBucket(ENTITY_TYPE_PLAYER).byte_size);
	free(buff);
	*/
	std::cin.get();

	return 0;


	
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
