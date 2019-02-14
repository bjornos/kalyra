#include <unistd.h>
#include <gtest/gtest.h>

#include "releaseComponent.hh"
#include "packageRecipe.hh"
#include "firmwareRelease.hh"

using namespace std;

#define TEST_RECIPE_NAME "test"
#define TEST_RECIPE_URL "https://github.com/bjornos/kalyra.git"
#define TEST_RECIPE_ROOT "kalyra"
#define TEST_RECIPE_REVISION "R1.4.5"
#define TEST_RECIPE_TARGET "default.target"

#define TEST_TARGET_1 "target one"
#define TEST_TARGET_2 "target two"
#define TEST_TARGET_3 "target three"

static const char* testRecipe =
"{"
	"\"package\":	{"
		"\"name\":	   \"" TEST_RECIPE_NAME     "\","
		"\"url\":	   \"" TEST_RECIPE_URL      "\","
		"\"root\":	   \"" TEST_RECIPE_ROOT     "\","
		"\"revision\": \"" TEST_RECIPE_REVISION "\","
		"\"target\":   \"" TEST_RECIPE_TARGET   "\","
		"\"depends\":  \"testRecipe2\" "
	"},"
	"\"default.target\":	"
        "["
          "\"" TEST_TARGET_1 "\","
          "\"" TEST_TARGET_2 "\","
          "\"" TEST_TARGET_3 "\""
        "],"
    "\"override.target\":	[ \"make clean\" ]"
"}";

class packageRecipeTest : public ::testing::Test {
public:
};

TEST_F(packageRecipeTest, testPackageRecipeParse)
{
    auto recipe(unique_ptr<packageRecipe>(new packageRecipe("testRecipe", "", "")));

    istringstream r(testRecipe);
    packageRecipe::parseRecipe(r, recipe);

    EXPECT_STREQ(TEST_RECIPE_NAME, recipe->getName().c_str());
    EXPECT_STREQ(TEST_RECIPE_REVISION, recipe->getRev().c_str());
    EXPECT_STREQ(TEST_RECIPE_URL, recipe->getUrl().c_str());
    EXPECT_STREQ(TEST_RECIPE_ROOT, recipe->getRoot().c_str());
    EXPECT_STREQ(TEST_RECIPE_TARGET, recipe->getTarget().c_str());
}

TEST_F(packageRecipeTest, testPackageRecipeParseOverride)
{
    constexpr auto overrideRevision = "R6.2.4";
    constexpr auto overrideTarget = "override.target";

    auto recipe(unique_ptr<packageRecipe>(new packageRecipe("testRecipe", overrideRevision, overrideTarget)));

    istringstream r(testRecipe);
    packageRecipe::parseRecipe(r, recipe);

    EXPECT_STRNE(TEST_RECIPE_REVISION, recipe->getRev().c_str());
    EXPECT_STREQ(overrideRevision, recipe->getRev().c_str());

    EXPECT_STRNE(TEST_RECIPE_TARGET, recipe->getTarget().c_str());
    EXPECT_STREQ(overrideTarget, recipe->getTarget().c_str());
}

TEST_F(packageRecipeTest, testPackageRecipeCmdList)
{
    auto recipe(unique_ptr<packageRecipe>(new packageRecipe("testRecipe","","")));

    istringstream r(testRecipe);
    packageRecipe::parseRecipe(r, recipe);

    auto cmd = recipe->getCmdList();
    EXPECT_STREQ(TEST_TARGET_1, cmd[0].c_str());
    EXPECT_STREQ(TEST_TARGET_2, cmd[1].c_str());
    EXPECT_STREQ(TEST_TARGET_3, cmd[2].c_str());
}
