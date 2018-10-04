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

int main(int argc, char *argv[])
{
    const cJSON* manifest;
    InputParser cmdOptions(argc, argv);
    bool fetchOnly = false;
    bool generateOnly = false;

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

    cout << "All good." << endl << endl;

    cout << "Release: " << termcolor::yellow << fwrt->getName() << " " << fwrt->getRelease() \
        << " " << fwrt->getStage() <<  fwrt->getBuild() << termcolor::reset << endl;
    cout << "Release Path: " << termcolor::yellow << "<not yet implemented>" << termcolor::reset << endl << endl;

    cout << "Generating build scripts...." << endl;

#if defined(_WIN32) || defined(_WIN64)
    if (!CreateDirectory(BUILDDIR, NULL) && (GetLastError() != ERROR_ALREADY_EXISTS)) {
        cerr << termcolor::red << "Failed to create build directory." << termcolor::reset << endl;
        return EXIT_FAILURE;
    }
#else
    struct stat st;
    if (stat(BUILDDIR, &st) != 0)
    {
        if (mkdir(BUILDDIR, 0755) == -1) {
            cerr << termcolor::red << "Failed to create build directory." << termcolor::reset << endl;
            return EXIT_FAILURE;
        }
    }
#endif

    scriptGenerator::fetch(fwrt);

    scriptGenerator::build(fwrt);

    if (generateOnly)
        return EXIT_SUCCESS;

    std::system(SCRIPT_FETCH_CMD);

    if (fetchOnly)
        return EXIT_SUCCESS;

    std::system(SCRIPT_BUILD_CMD);

    //cout << "Copying release to server..." << endl;

	return EXIT_SUCCESS;
}
