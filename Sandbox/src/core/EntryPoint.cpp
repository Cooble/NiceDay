#include "Sandbox.h"

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
	

	return std::string(buff,size);
}

void createRandomNBT(NBT& nbt, int deep)
{
	for (int i = 0; i < 10; ++i)
	{
		switch (std::rand() % 5)
		{
		case 0:
			nbt.set<int8_t>(nd::temp_string(getName()), (char)((std::rand() % 127)+1));
			break;
		case 1:
			nbt.set<uint32_t>(nd::temp_string(getName()), std::rand() % std::numeric_limits<uint32_t>::max());
			break;
		case 2:
			nbt.set<uint64_t>(nd::temp_string(getName()), std::rand() % std::numeric_limits<uint64_t>::max());
			break;
		case 3:
			nbt.set<std::string>(nd::temp_string(getName()), getRandomStringName());
			break;
		case 4:
			
			if (deep++ < 3) {
				createRandomNBT(nbt.get<NBT>(nd::temp_string(getName())),deep);
			}
			break;
		}
		}
	}


bool equalsNBT(NBT& one,NBT& two)
{
	for (auto& t : one.m_strings) {
		std::string& first = std::string(t.first);
		const char* c = first.c_str();
		std::string& oneoo = t.second;
		std::string& second = two.m_strings[first];
		if (oneoo != second)
			return false;
	}
	for (auto& t : one.m_ints)
		if (two.m_ints[t.first] != t.second)
			return false;
	for (auto& t : one.m_longs)
		if (two.m_longs[t.first] != t.second)
			return false;
	for (auto& t : one.m_nbts)
		if (!equalsNBT(two.m_nbts[t.first], t.second))
			return false;
	return true;
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

#ifndef ND_TEST
int main()
{
	ND_PROFILE_BEGIN_SESSION("start", "start.json");
	Log::init();
	
	Sandbox game;
	ND_PROFILE_END_SESSION();
	game.start();

	std::cin.get();

	return 0;
}
#endif
