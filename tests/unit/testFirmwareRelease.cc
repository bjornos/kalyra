#include <unistd.h>
#include <gtest/gtest.h>

#include "releaseComponent.hh"
#include "packageRecipe.hh"
#include "firmwareRelease.hh"

//using namespace std;

constexpr auto RELEASE_NAME = "testrelease";
constexpr auto RELEASE_NUMBER = "1.0";
constexpr auto RELEASE_STAGE = "test";
constexpr auto RELEASE_BUILD = "12";
constexpr auto RELEASE_ENV = "TEST=y";
constexpr auto RELEASE_PATH = "/tmp/test";

constexpr auto TEST_RECIPE_NAME = "test recipe";
constexpr auto TEST_RECIPE_REV = "R1.0.0";
constexpr auto TEST_RECIPE_TARGET = "test-target";

class firmwareReleaseTest : public ::testing::Test {
public:
};

TEST_F(firmwareReleaseTest, testReleaseConstructor)
{
    std::vector<std::string> commands {"", "", ""};
    auto testComponent(std::unique_ptr<releaseComponent>(new releaseComponent(commands, commands, commands)));

    std::vector<std::unique_ptr<packageRecipe>> testRecipes;

    auto fw(std::unique_ptr<firmwareRelease>(new firmwareRelease(RELEASE_NAME,RELEASE_NUMBER,RELEASE_STAGE,RELEASE_BUILD,
        RELEASE_PATH,RELEASE_ENV, move(testRecipes), move(testComponent))));

    EXPECT_STREQ(RELEASE_NAME, fw->getName().c_str());
    EXPECT_STREQ(RELEASE_NUMBER, fw->getRelease().c_str());
    EXPECT_STREQ(RELEASE_STAGE, fw->getStage().c_str());
    EXPECT_STREQ(RELEASE_BUILD, fw->getBuild().c_str());
    EXPECT_STREQ(RELEASE_ENV, fw->getEnv().c_str());
}

TEST_F(firmwareReleaseTest, testReleaseRecipes)
{
    constexpr auto numRecipes = 64;

    std::vector<std::string> commands {"", "", ""};
    auto testComponent(std::unique_ptr<releaseComponent>(new releaseComponent(commands, commands, commands)));

    std::vector<std::unique_ptr<packageRecipe>> testRecipes;
    for (int r = 0; r < numRecipes; r++) {
        auto p(std::unique_ptr<packageRecipe>(new packageRecipe(TEST_RECIPE_NAME + std::to_string(r), TEST_RECIPE_REV + std::to_string(r),
        TEST_RECIPE_TARGET + std::to_string(r))));
        testRecipes.emplace_back(std::move(p));
    }

    auto fw(std::unique_ptr<firmwareRelease>(new firmwareRelease(RELEASE_NAME,RELEASE_NUMBER,RELEASE_STAGE,RELEASE_BUILD,
        RELEASE_PATH,RELEASE_ENV, std::move(testRecipes), std::move(testComponent))));

    // Test hasRecipe()
    for (int r = 0; r < numRecipes; r++) {
        EXPECT_TRUE( fw->hasRecipe(TEST_RECIPE_NAME + std::to_string(r)));
    }

    // test getRecipes()
    int cnt = 0;
    for (auto &r : fw->getRecipes()) {
        const auto name(TEST_RECIPE_NAME + std::to_string(cnt++));
        EXPECT_STREQ(name.c_str(), r->getName().c_str());
    }
    EXPECT_EQ(numRecipes, cnt);
}