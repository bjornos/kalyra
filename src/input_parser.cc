#include <sstream>
#include <fstream>

#include "input_parser.hh"

using namespace std;

const std::string& InputParser::getCmdOption(const std::string &option) const
{
    std::vector<std::string>::const_iterator itr;

    itr =  std::find(this->tokens.begin(), this->tokens.end(), option);

    if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
        return *itr;
    }

    static const std::string empty_string("");

    return empty_string;
}

bool InputParser::cmdOptionExists(const std::string &option) const
{
    return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
}

const string& InputParser::get_manifest()
{
    manifest.assign(getCmdOption("-m"));

    if (manifest.empty()) {
        manifest.assign(getCmdOption("--manifest"));
    }

    if (manifest.empty()) {
        // No manifest provided. Try with previously selected manifest, if any.
        ifstream mFile(".kalyra-manifest");
        if (mFile.is_open()) {
            getline(mFile, manifest);
            mFile.close();
        }
    } else {
        // Save manifest as default for this build tree
        ofstream mFile (".kalyra-manifest");
        if (mFile.is_open()) {
            mFile << manifest;
            mFile.close();
        }
    }

    return manifest;
}

bool InputParser::alwaysYes()
{
    return optionAlwaysYes;
    return cmdOptionExists("--yes");
}

bool InputParser::clean()
{
    return optionClean;
}

bool InputParser::fetchOnly()
{
    return optionFetchOnly;
}

bool InputParser::buildOnly()
{
     return optionBuildOnly;
}

bool InputParser::updateOnly()
{
    return optionUpdateOnly;
}

const string& InputParser::getBuildSingle()
{
    if (optionBuildOnly) {
        buildSingle.assign(getCmdOption(OPT_SHORT_BUILD));
        if (buildSingle.empty()) {
            buildSingle.assign(getCmdOption(OPT_LONG_BUILD));
        }

        // Handle the case where next command line option could be treated
        // as recipe argument
        if (!buildSingle.empty() && buildSingle[0] == '-') {
            buildSingle.assign("");
        }
    }

    return buildSingle;
}

const string& InputParser::getFetchSingle()
{
    if (optionFetchOnly) {
        fetchSingle.assign(getCmdOption(OPT_SHORT_FETCH));
        if (fetchSingle.empty()) {
            fetchSingle.assign(getCmdOption(OPT_LONG_FETCH));
        }

        // Handle the case where next command line option could be treated
        // as recipe argument
        if (!fetchSingle.empty() && fetchSingle[0] == '-') {
            fetchSingle.assign("");
        }
    }

    return fetchSingle;
}

const string& InputParser::getUpdateSingle()
{
    // Doing an update does not trigger any builds unless implicitly specified
    if (optionBuildOnly)
        optionFetchOnly = false;

    updateSingle.assign(getCmdOption(OPT_SHORT_UPDATE));

    if (updateSingle.empty()) {
        updateSingle.assign(getCmdOption(OPT_LONG_UPDATE));
    }

    if (!updateSingle.empty() && updateSingle[0] == '-')
        updateSingle.assign("");
    
    return updateSingle;
}


bool InputParser::showRecipes()
{
    return optionShowRecipes;
}

bool InputParser::generateOnly()
{
    return optionGenerateOnly;
}

bool InputParser::release_build()
{
    return optionRelease;
}

bool InputParser::showHelp()
{
    return optionShowHelp;
}


InputParser::InputParser(int& argc, char** argv)
{
    for (int i=1; i < argc; ++i) {
        this->tokens.push_back(std::string(argv[i]));
    }

    optionClean = cmdOptionExists("-c") || cmdOptionExists("--clean");
    optionShowRecipes = cmdOptionExists(OPT_SHORT_RECIPES) || cmdOptionExists(OPT_LONG_RECIPES);

    optionRelease = cmdOptionExists("--release");
    optionShowHelp = cmdOptionExists("-h") || cmdOptionExists("--help");
    optionAlwaysYes = cmdOptionExists("--yes");

    optionFetchOnly = cmdOptionExists(OPT_SHORT_FETCH) || cmdOptionExists(OPT_LONG_FETCH);
    optionUpdateOnly = cmdOptionExists(OPT_SHORT_UPDATE) || cmdOptionExists(OPT_LONG_UPDATE);
    optionGenerateOnly = cmdOptionExists("-g") || cmdOptionExists("--generate");

    if (optionGenerateOnly || optionFetchOnly ||optionUpdateOnly) {
        optionBuildOnly = false;
    } else  {
        optionBuildOnly = cmdOptionExists(OPT_SHORT_BUILD) || cmdOptionExists(OPT_LONG_BUILD);
    }

}

InputParser::~InputParser()
{
}
