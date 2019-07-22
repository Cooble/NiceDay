#pragma once
#include "ndpch.h"
#include "Game.h"

#include <bitset>
#include <climits>

#include "world/entity/EntityRegistry.h"
#include "world/entity/entities.h"
#include "world/entity/entity_datas.h"
#include "nbt/NBT.h"
#include "physShapes.h"
#include "metaprogramming/Header.h"
#include "world/ChunkLoader.h"

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

void createRandomNBT(NBT& nbt, int size = 0);
bool equalsNBT(NBT& one, NBT& two);

/*template<>
struct Caller<int>
{
	void operator()(int idx, int karel)
	{
		ND_INFO("Cus picus idx {}, arg {}", idx, karel);
	}
};
template <>
inline void looper<0, int>(int args)
{

}*/
struct ChunkQuadroo
{
	std::pair<int, int> src[4];

	std::pair<int, int>& operator[](size_t index)
	{
		return src[index];
	}
	
};
inline std::ostream& operator<<(std::ostream& stream,const ChunkQuadroo& q)
{
	stream << "[";
	for (auto& i : q.src)
	{
		stream << "(" << i.first << ", " << i.second << ") ";
	}
	stream << "\n";
	return stream;
}

static ChunkQuadroo computeQuadroo(int x, int y)
{
	ChunkQuadroo out;
	int chunkX = x >> 5;
	int chunkY = y >> 5;
	if (x < 32 / 2)
	{
		if (y < 32 / 2)
		{
			out[0] = std::make_pair(chunkX, chunkY);
			out[1] = std::make_pair(chunkX - 1, chunkY);
			out[2] = std::make_pair(chunkX, chunkY - 1);
			out[3] = std::make_pair(chunkX - 1, chunkY - 1);
		}
		else
		{
			out[0] = std::make_pair(chunkX, chunkY + 1);
			out[1] = std::make_pair(chunkX - 1, chunkY + 1);
			out[2] = std::make_pair(chunkX, chunkY);
			out[3] = std::make_pair(chunkX - 1, chunkY);
		}
	}
	else
	{
		if (y < 32 / 2)
		{
			out[0] = std::make_pair(chunkX + 1, chunkY);
			out[1] = std::make_pair(chunkX, chunkY);
			out[2] = std::make_pair(chunkX + 1, chunkY - 1);
			out[3] = std::make_pair(chunkX, chunkY - 1);
		}
		else
		{
			out[0] = std::make_pair(chunkX + 1, chunkY + 1);
			out[1] = std::make_pair(chunkX, chunkY + 1);
			out[2] = std::make_pair(chunkX + 1, chunkY);
			out[3] = std::make_pair(chunkX, chunkY);
		}
	}
	return out;
	
}

int main()
{
	using namespace Phys;
	Log::Init();
	
	/*int josef = 0xab;
	looper<10>(josef);



	ND_WAIT_FOR_INPUT;
	return 0;*/
/*	std::vector<Vect> ints = {
		{0, 0},
		{1, 0},
		{0.5, 1}
	};
	std::vector<Vect> ints2 = {
		{1, 0},
		{2, 0},
		{1, 1},
		{1, 1},
	};
	auto a = Phys::Polygon(ints);
	auto b = Phys::Polygon(ints2);

	ND_INFO("intersects polygons: {}",isIntersects(a, b));
	std::cout << intersects(a, b) << "\n";
	//ND_INFO("intersects polygons: {}",);




	Vect p0 = {1, 0};
	Vect p1 = {0, 0};
	Vect p2 = {1, 0.5};
	Vect p3 = {0, 0.5};

	Vect collisionPoint = intersectLines(p1, p0, p2, p3);

	ND_INFO("intersection {},{}, isabcusses {}", collisionPoint.x, collisionPoint.y,
	        isIntersectAbscisses(p1, p0, p3, p2));

	std::cin.get();
	return 0;
	*/
	/*auto stream = std::fstream("src/data.example", std::ios::out | std::ios::binary);
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
	second.~NBT();*/


	/*void* buff = malloc(sizeof(EntityPlayer));
	EntityRegistry::get().createInstance(ENTITY_TYPE_PLAYER, buff);

	auto* karle = (WorldEntity*)buff;
	auto* pPlayer = (EntityPlayer*) buff;

	ND_INFO("ID of player is: {}", pPlayer->getEntityType());
	ND_INFO("ID of player is: {}", EntityRegistry::get().entityTypeToString(karle->getEntityType()));
	ND_INFO("size of player is: {}", EntityRegistry::get().getBucket(ENTITY_TYPE_PLAYER).byte_size);
	free(buff);
	
	std::cin.get();

	return 0;*/


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
