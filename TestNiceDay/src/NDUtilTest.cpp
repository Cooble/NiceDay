#include "NDTests.h"
#include "core/SUtil.h"
#include "core/NBT.h"
#include <nlohmann/json.hpp>

using namespace nd;

static void testNBT();
static void testSUtil();
static void testNBTSerialization();

int NDUtilTest()
{
	NDT_TRY(testNBT());
	NDT_TRY(testSUtil());
    NDT_TRY(testNBTSerialization());
	return 0;
}
static void testNBT()
{
    NBT n;

    n["karel"] = "josef";
    NDT_ASSERT(n["karel"].string() == "josef");

    n["one"] = 1;
    NDT_ASSERT_EQUAL((int)n["one"], 1);

    n["boo"] = true;
    NDT_ASSERT(n["boo"]);
    //saving test
    NBT::saveToFile("testing.nbt", n);
    NBT trsf;
    NBT::loadFromFile("testing.nbt", trsf);
    NDT_ASSERT_EQUAL(trsf, n);
}

static void testSUtil()
{
    {//replace with
        std::string s = "Something interesting is it not or not or something else not!";
        SUtil::replaceWith(s, 'o', 'u');
        NDT_ASSERT_EQUAL(s, "Sumething interesting is it nut ur nut ur sumething else nut!");
    }
    {
        std::string s = "Something interesting is it not or not or something else not!";
        SUtil::replaceWith(s, "not", "yessssir");
        NDT_ASSERT_EQUAL(s, "Something interesting is it yessssir or yessssir or something else yessssir!");
    }

    {//splitting
        const std::string line = ";This is line ;and this is not\nwhat we're going to do-->;nobody knows.;;";
        const std::vector<std::string> targetStringsWithBlanks
        {
            "",
            "This is line ",
            "and this is not",
            "what we're going to do-->",
            "nobody knows.",
            "",
            ""
        };
        const std::vector<std::string> targetStringsNoBlanks
        {
            "This is line ",
            "and this is not",
            "what we're going to do-->",
            "nobody knows.",
        };

        {
            std::vector<std::string> strings;
            SUtil::splitString(line, strings, ";\n");
            NDT_ASSERT_EQUAL(strings, targetStringsNoBlanks);
        }
        {
            auto splitter = SUtil::SplitIterator<false>(line, ";\n");
            std::vector<std::string> strings;
            for (; splitter; ++splitter)
                strings.emplace_back(*splitter);
            NDT_ASSERT_EQUAL(strings, targetStringsWithBlanks);
        }
        {
            auto splitter = SUtil::SplitIterator<true>(line, ";\n");
            std::vector<std::string> strings;
            for (; splitter; ++splitter)
                strings.emplace_back(*splitter);
            NDT_ASSERT_EQUAL(strings, targetStringsNoBlanks);
        }
        {
            const std::vector<std::string> targetStringsNoBlanksSemicolonsOnly
            {
                "This is line ",
                "and this is not\nwhat we're going to do-->",
                "nobody knows.",
            };
            auto splitter = SUtil::SplitIterator<true, char>(line, ';');
            std::vector<std::string> strings;
            for (; splitter; ++splitter)
                strings.emplace_back(*splitter);
            NDT_ASSERT_EQUAL(strings, targetStringsNoBlanksSemicolonsOnly);
        }
    }
}
static void testNBTSerialization()
{
    NBT n;
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


    NBT::saveToFile("hayaku.json", lis);
    NBT newlIs;
    NBT::loadFromFile("hayaku.json", newlIs);
    NDT_ASSERT(newlIs == lis);
    NDT_ASSERT(NBT::fromJson(lis.toJson()) == lis);
    NBT cop = lis;

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
    NDT_ASSERT(newlIs == lis);
}

