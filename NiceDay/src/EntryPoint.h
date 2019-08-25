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
#include "world/ChunkLoader.h"
#include "graphics/Effect.h"
#include "graphics/BlockTextureAtlas.h"

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

	
	Game game;
	game.init();
	game.start();
	std::cin.get();

	return 0;
}
