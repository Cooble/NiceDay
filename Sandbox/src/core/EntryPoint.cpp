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
	return "kareljeposuk";
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
	
	ND_PROFILE_BEGIN_SESSION("start", "start.json");
	Log::init();
	Sandbox game;

	/*NBT n;
	n = 12;
	NBT mapka;
	mapka["twelfe"] = n;
	mapka["something else"] = n;
	int twe;
	std::string twe1="hhheeee";
	mapka.save("blemc", twe);
	mapka.save("blemc", twe1);
	mapka.load("blemc", twe1, std::string("hohhoo"));
	mapka.load("blemc2", twe1, std::string("hohhoo"));

	NBT listik;
	listik[0] = 1;
	listik[2] = 2.5;
	listik[2] = 3;
	mapka["listik"] = listik;
	ND_INFO("\n"+mapka.dump());

	int twelfe = int(listik[0]);
	double d0 = int(listik[0]);
	double d1 = double(listik[2]);
	double d2 = (double)listik[2];
	NBT ssss = "whatever";
	ssss.string() += " blemc ";
	ssss = "nwever";
	NBT lis;
	for (int i = 0; i < 5; ++i)
	{
		lis.push_back("Help " + std::to_string(i));
	}
	lis.push_back(3.0f);
	lis.push_back(true);
	lis.push_back(std::numeric_limits<uint64_t>::max());
	lis.push_back(lis);
	lis.emplace_back("emplaced stringooo hhoho");
	lis.push_back(mapka);
	
	
	ND_INFO("\n" + lis.dump());
	NBT::saveToFile("hayaku.json", lis);
	NBT newlIs;
	NBT::loadFromFile("hayaku.json", newlIs);
	bool equ = newlIs==lis;
	
	ND_INFO("nbt2 through json\n" + NBT::fromJson(lis.toJson()).dump());

	bool eq = NBT::fromJson(lis.toJson()) == lis;
	NBT cop = lis;
	//cop.push_back(5);
	bool eqne = NBT::fromJson(cop.toJson()) == lis;

	std::fstream stream;
	stream.open("cruci.dat", std::ios::binary | std::ios::out);
	BinarySerializer::write(lis, std::bind(&std::fstream::write, &stream, std::placeholders::_1, std::placeholders::_2));
	stream.flush();
	stream.close();
	NBT newLis;
	std::fstream stream2;
	stream2.open("cruci.dat", std::ios::binary | std::ios::in);
	BinarySerializer::read(newLis, std::bind(&std::fstream::read, &stream2, std::placeholders::_1, std::placeholders::_2));
	stream2.close();
	ASSERT(newlIs == lis, "shit");*/
	ND_PROFILE_END_SESSION();
	game.start();

	std::cin.get();

	return 0;
}
#endif
