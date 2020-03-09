#pragma once

#include <algorithm>
#include <vector>
#include <cstdlib>
#include <string>
#include <tuple>

#define OPT_SHORT_BUILD   "-b"
#define OPT_LONG_BUILD    "--build"
#define OPT_DESC_BUILD    "Build recipe only. If no recipe is stated, all recipes are built."

#define OPT_SHORT_RECIPES  "-r"
#define OPT_LONG_RECIPES   "--recipes"
#define OPT_DESC_RECIPES   "Show avaiable recipes for current manifest"

#define OPT_SHORT_FETCH   "-f"
#define OPT_LONG_FETCH    "--fetch"
#define OPT_DESC_FETCH    "Fetch recipe repository only. No argument means all recipes."

#define OPT_FWRT           "--fwrt"

#define OPT_SHORT_UPDATE   "-u"
#define OPT_LONG_UPDATE    "--update"
#define OPT_DESC_UPDATE    "Update sources with latest changes on remote repository for appointed branch. If no recipe is stated, all repositories are updated."

#define OPT_SHORT_GENERATE   "-g"
#define OPT_LONG_GENERATE    "--generate"
#define OPT_DESC_GENERATE    "Generate build scripts only."


class InputParser 
{
    public:
        InputParser(int &argc, char **argv);
        ~InputParser();

        const std::string& getCmdOption(const std::string &option) const;
        bool cmdOptionExists(const std::string &option) const;

        const std::string& get_manifest();
        bool showHelp();
        bool alwaysYes();
        bool clean();
        bool fetchOnly();
        bool buildOnly();
        bool showRecipes();
        bool generateOnly();
        bool updateOnly();
        bool FWRT();

        const std::string& getFetchSingle();
        const std::string& getBuildSingle();
        const std::string& getUpdateSingle();

    private:
        std::vector <std::string> tokens;

        std::string fetchSingle;
        std::string buildSingle;
        std::string updateSingle;
        std::string manifest;
        
        bool optionClean;
        bool optionFetchOnly;
        bool optionUpdateOnly;
        bool optionBuildOnly;
        bool optionGenerateOnly;
        bool optionShowRecipes;
        bool optionShowHelp;
        bool optionAlwaysYes;
        bool optionFWRT;
};
