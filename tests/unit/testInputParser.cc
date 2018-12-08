#include <unistd.h>
#include <gtest/gtest.h>

#include "inputParser.hh"

using namespace std;

#define ARRAY_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

class inputParserTest : public ::testing::Test {
public:
};

TEST_F(inputParserTest, testShortOptions)
{
    char arg0[] = "ff";
    char arg1[] = "-h";
    char arg2[] = "-b";

    char* argv[] = { arg0, arg1, arg2 };
    int argc = ARRAY_ELEMENTS(argv);

    InputParser opt(argc, argv);

    EXPECT_TRUE(opt.cmdOptionExists(arg1));
    EXPECT_TRUE(opt.cmdOptionExists(arg2));

    EXPECT_FALSE(opt.cmdOptionExists("--h"));
    EXPECT_FALSE(opt.cmdOptionExists("1"));
    EXPECT_FALSE(opt.cmdOptionExists("*"));
}

