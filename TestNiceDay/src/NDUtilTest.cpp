#include "NDTests.h"
#include "core/SUtil.h"
#include "core/NBT.h"
static void testNBT();
static void testSUtil();

int NDUtilTest()
{
	NDT_TRY(testNBT());
	NDT_TRY(testSUtil());
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

