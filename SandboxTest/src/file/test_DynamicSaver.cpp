#include "test_DynamicSaver.h"
#include "files/DynamicSaver.h"
#include "core/NBT.h"

std::string genRanString(int size)
{
	std::string out;
	out.reserve(size);
	for (int i = 0; i < size; ++i)
		out += (char)((int)'0' + std::rand() % 64);
	return out;
}

NBT genRanNBT(int layers,int size)
{

	NBT out;
	for (int i = 0; i < size; ++i)
	{
		auto s = genRanString( 1 + std::rand() % 25);
		NBT& e=out[s];
		
		switch ((i+std::rand()%2)%7)
		{
		case 0:
			e = std::rand();
			break;
		case 1:
			e = (uint64_t)std::rand()* std::rand();
			break;
		case 2:
			e = genRanString(1 + std::rand() % 25);
			break;
		case 3:
			e = (bool)(std::rand()&1);
			break;
		case 4:
		case 5:
			if (layers != 0)
				e = genRanNBT(layers - 1, size);
			break;
		}
	}
	return out;
}

void writeData(DynamicSaver& saver, size_t size, unsigned seed)
{
	uint32_t data;
	for (int i = 0; i < size; ++i)
	{
		data = (seed++);
		saver.write((char*)&data, sizeof(uint32_t));
	}
}
bool checkData(DynamicSaver& saver, size_t size, unsigned seed)
{
	uint32_t data;
	for (int i = 0; i < size; ++i)
	{
		saver.read((char*)&data, sizeof(uint32_t));
		if (data != seed++)
			return false;
	}
	return true;
}
void writeDataNBT(DynamicSaver& saver, unsigned seed)
{
	auto func = std::bind(&DynamicSaver::writeI, &saver, std::placeholders::_1, std::placeholders::_2);
	std::srand(seed);
	BinarySerializer::write(genRanNBT(5, 10), func);
	
}
bool checkDataNBT(DynamicSaver& saver, unsigned seed)
{
	NBT t;
	auto func = std::bind(&DynamicSaver::readI, &saver, std::placeholders::_1, std::placeholders::_2);
	BinarySerializer::read(t,func);
	std::srand(seed);
	return t == genRanNBT(5, 10);
}
int srando[10];
void testik(DynamicSaver& saver)
{
	for (int& i : srando)
	{
		i = std::rand();
	}
	saver.beginSession();

	int lastSeed = -1;
	int eger;
	for (int i = 0; i < 10; ++i)
	{
		int seed =(srando[i] & 255)*i+10;
		if((seed&3)==0)
		{
			saver.endSession();
			saver.beginSession();
		}

		saver.setWriteChunkID(seed);
		for (int i = 0; i < 5; ++i)
		{
			writeDataNBT(saver, seed*123*i+5);
			eger = seed * 123 * i + 5;
		}
		if(lastSeed!=-1)
		{
			saver.setReadChunkID(lastSeed);
			checkDataNBT(saver, eger);
		}
		lastSeed = seed;
	}
	for (int i = 0; i < 10; ++i)
	{
		int seed = (srando[i] & 255) * i + 10;
		if ((seed & 3) == 0)
		{
			saver.setWriteChunkID(seed);
			for (int i = 0; i < 15; ++i)
			{
				writeDataNBT(saver, seed * 123 * i + 6);
			}
		}
	}
	for (int i = 0; i < 10; ++i)
	{
		int seed = (srando[9-i] & 255) * (9-i) + 10;
		if ((seed & 3) == 2)
		{
			saver.endSession();
			saver.beginSession();
		}
		ASSERT(saver.setReadChunkID(seed),"");
		if ((seed & 3) == 0)
		{
			for (int i = 0; i < 15; ++i)
			{
				checkDataNBT(saver, seed * 123 * i + 6);
			}
		}
		else {
			for (int i = 0; i < 5; ++i)
			{
				ASSERT(checkDataNBT(saver, seed * 123 * i + 5), "");
			}
		}
	}
}

int nd_test_DynamicSaver()
{
	//ND_INFO("testing start...");

	//DynamicSaver saver("dynamicSaverTest.dat");
	
	//saver.init();
	//testik(saver);
	//ND_INFO("testing done");
	return 0;
}
