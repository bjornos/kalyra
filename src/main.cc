#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include<sys/stat.h>
#include<sys/types.h>

// script generator
#include <iostream>
#include <fstream>  

#include "cJSON/cJSON.h"
#include "termcolor/termcolor.hpp"

#include "packageRecipe.hh"
#include "releaseComponent.hh"
#include "firmwareRelease.hh"
#include "scriptGenerator.hh"

using namespace std;

constexpr auto KALYRA_MAJOR = 0;
constexpr auto KALYRA_MINOR = 2;
constexpr auto KALYRA_SUB = 1;

// for unit tests
const char* manifestFile = 
"{"
	"\"product\": \"projectX\","
	"\"release\": \"1.0.0\","
	"\"stage\": \"alpha\""
"}";

constexpr auto TAG_PRODUCT = "product";
constexpr auto TAG_RELEASE = "release";
constexpr auto TAG_STAGE = "stage";
constexpr auto TAG_BUILD = "build";
constexpr auto TAG_PACKAGES = "packages";

enum {
	ALPHA,
	BETA,
	RC,
	FINAL
} RELEASE_STAGE;

// this class was found on stackoverflow
// https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c
class InputParser{
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }
        /// @author iain
        const std::string& getCmdOption(const std::string &option) const {
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }
        /// @author iain
        bool cmdOptionExists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        std::vector <std::string> tokens;
};

int createDir(const string& dir)
{
//FIXME: look into c++17 for cross platform solution
#if defined(_WIN32) || defined(_WIN64)
    if (!CreateDirectory(dir, NULL) && (GetLastError() != ERROR_ALREADY_EXISTS)) {
        return -1;
    } else {
        return 0;
    }
#else
    struct stat st;
    if (stat(dir.c_str(), &st)) {
        return mkdir(dir.c_str(), 0755);
    } else {
        return 1;
    }
#endif
}

const cJSON* manifestGetValue(const cJSON* recipe, string tag)
{
    const cJSON* entry;

	if (cJSON_HasObjectItem(recipe, tag.c_str())) {
        entry = cJSON_GetObjectItemCaseSensitive(recipe, tag.c_str());
    } else {
    	cerr << "Warning: Could not find component '" << tag << "'." << endl;
    	entry = cJSON_CreateString("Unknown");
    }

    return entry;
}

void manifestLoadHeader(const cJSON*& m, const string& manifest)
{
    std::ifstream t(manifest);
    std::stringstream buffer;

    buffer << t.rdbuf();

    m = cJSON_Parse(buffer.str().c_str());

    if (m == NULL) {
        // TODO: add file exists check and report
        const char* errPtr = cJSON_GetErrorPtr();
        if (errPtr != NULL) {
            cout << "[DBG] Error before: " << errPtr << endl;
        }
        throw std::invalid_argument("Error parsing manifest.");
    }
}

void manifestLoadTargets(unique_ptr<firmwareRelease>& fwrt, const cJSON* manifest)
{
    const cJSON* package;
    const cJSON* packages;

    packages = cJSON_GetObjectItemCaseSensitive(manifest, "packages");
    cJSON_ArrayForEach(package, packages)
    {
        string revOverride("");
        string targetOverride("");

        cJSON *recipe = cJSON_GetObjectItemCaseSensitive(package, "name");
        cJSON *rev = cJSON_GetObjectItemCaseSensitive(package, "revision");
        cJSON *target = cJSON_GetObjectItemCaseSensitive(package, "target");

        if (!cJSON_IsString(recipe) || (recipe->valuestring == NULL)) {
            throw std::invalid_argument("Error parsing targets.");
        }

        if (cJSON_IsString(rev) && (rev->valuestring != NULL))
            revOverride = rev->valuestring;

        if (cJSON_IsString(target) && (target->valuestring != NULL))
            targetOverride = target->valuestring;

        auto p(unique_ptr<packageRecipe>(new packageRecipe(recipe->valuestring, revOverride, targetOverride)));

        fwrt->recipes.emplace_back(move(p));
    }
}

void manifestLoadComponents(unique_ptr<firmwareRelease>& fwrt, const cJSON* manifest)
{
    const cJSON* component;
    vector<string> preCommands;
    vector<string> postCommands;
    vector<string> releaseFiles;

    auto components = cJSON_GetObjectItemCaseSensitive(manifest, "release-components");

    const auto path = manifestGetValue(components, "release-path");

    auto section = cJSON_GetObjectItemCaseSensitive(components, "pre-commands");
    if (section != NULL) {
        cJSON_ArrayForEach(component, section)
        {
            preCommands.emplace_back(component->valuestring);
        }
    }

    section = cJSON_GetObjectItemCaseSensitive(components, "post-commands");
    if (section != NULL) {
        cJSON_ArrayForEach(component, section)
        {
            postCommands.emplace_back(component->valuestring);
        }
    }

    for (auto& target : fwrt->recipes) {
        const cJSON* releaseFile;
        auto t = cJSON_GetObjectItem(components, target->getName().c_str());
        if (t != NULL) {
            cJSON_ArrayForEach(releaseFile, t)
            {
                 // FIXME!!! windows make things complicated with their silly approach of \
                //releaseFiles.emplace_back(target->getRoot() +"/" + releaseFile->valuestring);
                releaseFiles.emplace_back(target->getRoot() +"\\" + releaseFile->valuestring);
            }
        }
    }

    auto rc(unique_ptr<releaseComponent>(new releaseComponent(preCommands, postCommands, releaseFiles, path->valuestring)));

    fwrt->releaseComponents = move(rc);
}

int main(int argc, char *argv[])
{
    const cJSON* manifest;
    InputParser cmdOptions(argc, argv);
    bool fetchOnly = false;
    bool generateOnly = false;
    bool releaseOnly = false;

    cout <<  termcolor::cyan << "Kalyra Build System v" << KALYRA_MAJOR <<"." << KALYRA_MINOR << "." << KALYRA_SUB << termcolor::reset << endl;

    if (cmdOptions.cmdOptionExists("--help")){
    	cout << "Available command options:" << endl;
        cout << "-m, --manifest <name> : Project manifest file (mandatory)." << endl;
        cout << "-g                    : Generate build scripts only" << endl;
        cout << "-f                    : Fetch targets only" << endl;
        cout << "-c <target, ..>       : " << termcolor::red << "[Not Implemented Yet]" << termcolor::reset << " Compile/Assemble firmware only. No target means all." << endl;
    	return EXIT_SUCCESS;
    }

    auto fileName(cmdOptions.getCmdOption("-m"));
    if (fileName.empty()) {
        fileName.assign (cmdOptions.getCmdOption("--manifest"));
        if (fileName.empty()) {
            cerr <<  "Missing manifest file. Try --help" <<  endl;
    	    return EXIT_FAILURE;
        }
    }

    if (cmdOptions.cmdOptionExists("-f"))
        fetchOnly = true;

    if (cmdOptions.cmdOptionExists("-g"))
        generateOnly = true;

    if (cmdOptions.cmdOptionExists("-r"))
        releaseOnly = true;

    cout << "Processing " << fileName << "... ";

    try {
    	manifestLoadHeader(manifest, fileName);
    } catch (const exception& e) {
        cerr << termcolor::red <<e.what() << termcolor::reset << endl <<  "Abort!" << endl;
        return EXIT_FAILURE;
    }

    const auto product = manifestGetValue(manifest, TAG_PRODUCT);
    const auto release = manifestGetValue(manifest, TAG_RELEASE);
    const auto stage = manifestGetValue(manifest, TAG_STAGE);
    const auto build = manifestGetValue(manifest, TAG_BUILD);

    auto fwrt(unique_ptr<firmwareRelease>(new firmwareRelease(product->valuestring,
    	      release->valuestring, stage->valuestring, build->valuestring)));

    try {
        manifestLoadTargets(fwrt, manifest);
    } catch (const exception& e) {
        cerr << termcolor::red <<e.what() << termcolor::reset << endl <<  "Abort!" << endl;
            return EXIT_FAILURE;
    }

    for (auto& entry : fwrt->recipes) {
        try {
            packageRecipe::parseRecipe(entry);
        } catch (const exception& e) {
            cerr << termcolor::red << e.what() << termcolor::reset << endl;
            return EXIT_FAILURE;
        }

    }

    manifestLoadComponents(fwrt, manifest);

    cout << "All good." << endl << endl;

    cout << "Release: " << termcolor::yellow << fwrt->getName() << " " << fwrt->getRelease() \
        << " " << fwrt->getStage() <<  fwrt->getBuild() << termcolor::reset << endl;
    cout << "Release Path: " << termcolor::yellow << fwrt->releaseComponents->releasePath << termcolor::reset << endl << endl;

    cout << "Generating build scripts...." << endl;

    if (createDir(BUILDDIR) == -1) {
        cerr << termcolor::red << "Failed to create build directory." << termcolor::reset << endl;
        return EXIT_FAILURE;
    }

    scriptGenerator::fetch(fwrt);

    scriptGenerator::build(fwrt);

    scriptGenerator::release(fwrt);

    if (generateOnly)
        return EXIT_SUCCESS;

    std::system(SCRIPT_FETCH_CMD);

    if (fetchOnly)
        return EXIT_SUCCESS;

    if (!releaseOnly)
        std::system(SCRIPT_BUILD_CMD);

    cout << "Copying release to server..." << endl;

    if (createDir(fwrt->releaseComponents->releasePath) == -1) {
        cerr << termcolor::red << "Failed to create release directory." << termcolor::reset << endl;
        return EXIT_FAILURE;
    }

    std::system(SCRIPT_RELEASE_CMD);

	return EXIT_SUCCESS;
}
