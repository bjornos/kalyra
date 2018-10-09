#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "cJSON/cJSON.h"
#include "termcolor/termcolor.hpp"

#include "manifest.hh"
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

int main(int argc, char *argv[])
{
    const cJSON* manifest;
    InputParser cmdOptions(argc, argv);
    auto fetchOnly = false;
    auto generateOnly = false;
    auto releaseOnly = false;

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
        manifest::loadHeader(manifest, fileName);
    } catch (const exception& e) {
        cerr << termcolor::red <<e.what() << termcolor::reset << endl <<  "Abort!" << endl;
        return EXIT_FAILURE;
    }

    const auto product = manifest::getValue(manifest, TAG_PRODUCT);
    const auto release = manifest::getValue(manifest, TAG_RELEASE);
    const auto stage = manifest::getValue(manifest, TAG_STAGE);
    const auto build = manifest::getValue(manifest, TAG_BUILD);

    auto fwrt(unique_ptr<firmwareRelease>(new firmwareRelease(product->valuestring,
    	      release->valuestring, stage->valuestring, build->valuestring)));

    try {
        manifest::loadTargets(fwrt, manifest);
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

    manifest::loadComponents(fwrt, manifest);

    cout << "All good." << endl << endl;

    cout << "Release: " << termcolor::yellow << fwrt->getName() << " " << fwrt->getRelease() \
        << " " << fwrt->getStage() <<  fwrt->getBuild() << termcolor::reset << endl;
    cout << "Release Path: " << termcolor::yellow << fwrt->releaseComponents->releasePath << termcolor::reset << endl << endl;

    cout << "Generating build scripts...." << endl;

    if (createDir(scriptGenerator::BUILDDIR) == -1) {
        cerr << termcolor::red << "Failed to create build directory." << termcolor::reset << endl;
        return EXIT_FAILURE;
    }

    scriptGenerator::fetch(fwrt);

    scriptGenerator::build(fwrt);

    scriptGenerator::release(fwrt);

    if (generateOnly)
        return EXIT_SUCCESS;

    std::system(scriptGenerator::SCRIPT_FETCH_CMD);

    if (fetchOnly)
        return EXIT_SUCCESS;

    if (!releaseOnly)
        std::system(scriptGenerator::SCRIPT_BUILD_CMD);

    cout << "Copying release to server..." << endl;

    if (createDir(fwrt->releaseComponents->releasePath) == -1) {
        cerr << termcolor::red << "Failed to create release directory." << termcolor::reset << endl;
        return EXIT_FAILURE;
    }

    std::system(scriptGenerator::SCRIPT_RELEASE_CMD);

	return EXIT_SUCCESS;
}
