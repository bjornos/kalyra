#pragma once

#include <algorithm>
#include <vector>
#include <cstdlib>
#include <string>
#include <tuple>

#define OPT_BUILD_LONG   "--build"

#define OPT_UPDATE_SHORT "-u"
#define OPT_UPDATE_LONG  "--update"
#define OPT_UPDATE_DESC   "Update sources with latest changes on remote repository for appointed branch. If no recipe is stated, all repositories are updated."

class InputParser 
{
    public:
        InputParser(int &argc, char **argv);
        ~InputParser();

        const std::string& getCmdOption(const std::string &option) const;
        bool cmdOptionExists(const std::string &option) const;

        const std::string& getManifest();
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
