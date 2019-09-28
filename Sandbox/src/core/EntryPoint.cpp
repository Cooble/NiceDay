#include "EntryPoint.h"
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
			nbt.set<int8_t>(getName(), (char)((std::rand() % 127)+1));
			break;
		case 1:
			nbt.set<uint32_t>(getName(), std::rand() % std::numeric_limits<uint32_t>::max());
			break;
		case 2:
			nbt.set<uint64_t>(getName(), std::rand() % std::numeric_limits<uint64_t>::max());
			break;
		case 3:
			nbt.set<std::string>(getName(), getRandomStringName());
			break;
		case 4:
			
			if (deep++ < 3) {
				createRandomNBT(nbt.get<NBT>(getName()),deep);
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

int main()
{
	Sandbox game;
	game.start();

	std::cin.get();

	return 0;
}
