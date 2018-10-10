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

#if defined(_WIN32) || defined(_WIN64)
// this is so extra silly
#define cerr cout
#endif

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
    if (!CreateDirectory(dir.c_str(), NULL) && (GetLastError() != ERROR_ALREADY_EXISTS)) {
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

bool runScript(const char* cmd)
{
    auto cmdResult = std::system(cmd);
#if defined(_WIN32) || defined(_WIN64)
    if (cmdResult != 0) {
#else
    if (WEXITSTATUS(cmdResult) != 0) {
#endif
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    const cJSON* manifest;
    InputParser cmdOptions(argc, argv);
    string singleTarget;
    auto optFetchOnly = false;
    auto optGenerateOnly = false;
    auto optBuildOnly = false;
    auto optFwrt = false;
    auto optClean = false;

    cout <<  termcolor::cyan << "Kalyra Build System v" << KALYRA_MAJOR <<"." << KALYRA_MINOR << "." << KALYRA_SUB << termcolor::reset << endl;
    cout <<  "Firmware Factory: ";

    if (cmdOptions.cmdOptionExists("--help")){
    	cout << "Available command options:" << endl;
        cout << "-m, --manifest <name>   : Project manifest file (mandatory)." << endl;
        cout << "-g, --generate          : Generate build scripts only" << endl;
        cout << "-f, --fetch             : Fetch targets only" << endl;
        cout << "-c, --clean             : Clean working directory" << endl;
        cout << "-b, --build <recipe>    : Build recipe only. If no recipe is stated, all recipes are built." << endl;
        cout << "--fwrt                  : Firmware Release Tool. Generate a official release after building all components." << endl;
    	return EXIT_SUCCESS;
    }

    if (cmdOptions.cmdOptionExists("-c") || cmdOptions.cmdOptionExists("--clean")) {
        cout << "Clean up workspace..." << endl;
        // yes, very bad practise. Shall be removed when making the move to c++17
        return std::system("rm -rf " BUILDDIR);
    }

    auto fileName(cmdOptions.getCmdOption("-m"));
    if (fileName.empty()) {
        fileName.assign (cmdOptions.getCmdOption("--manifest"));
        if (fileName.empty()) {
            cerr <<  "Missing manifest file. Try --help" <<  endl;
    	    return EXIT_FAILURE;
        }
    }

    if (cmdOptions.cmdOptionExists("-f") || cmdOptions.cmdOptionExists("--fetch"))
        optFetchOnly = true;

    if (cmdOptions.cmdOptionExists("-g"))
        optGenerateOnly = true;

    if (cmdOptions.cmdOptionExists("--fwrt"))
        optFwrt = true;

    if (cmdOptions.cmdOptionExists("-b") || cmdOptions.cmdOptionExists("--build")) {
        optBuildOnly = true;
        singleTarget.assign(cmdOptions.getCmdOption("-b"));
        if (singleTarget.empty()) {
            singleTarget.assign(cmdOptions.getCmdOption("--build"));
        }
    }

    cout << "Processing " << fileName << "... " << endl << endl;

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
            cerr << termcolor::red << e.what() << termcolor::reset << endl << "Abort!" << endl;
            return EXIT_FAILURE;
        }

    }

    manifest::loadComponents(fwrt, manifest);

    if (optFwrt) {
        cout << "Release: " << termcolor::yellow << fwrt->getName() << " " << fwrt->getRelease() \
            << " " << fwrt->getStage() <<  fwrt->getBuild() << termcolor::reset << endl;
        cout << "Release Path: " << termcolor::yellow << fwrt->releaseComponents->releasePath \
            << termcolor::reset << endl << endl;
    }

    cout << "Generating build scripts...." << endl;

    if (createDir(BUILDDIR) == -1) {
        cerr << termcolor::red << "Failed to create build directory." << termcolor::reset << endl;
        return EXIT_FAILURE;
    }

    scriptGenerator::fetch(fwrt);

    scriptGenerator::build(fwrt, singleTarget);

    if (optFwrt)
        scriptGenerator::release(fwrt);

    if (optGenerateOnly)
        return EXIT_SUCCESS;

    if (!optBuildOnly) {
        // fetch may return non success - but it is not neccessarily critical
        runScript(SCRIPT_CMD_FETCH);

        if (optFetchOnly)
            return EXIT_SUCCESS;
    }

    if (!runScript(SCRIPT_CMD_BUILD)) {
        cerr << termcolor::red << "Abort!" << termcolor::reset << endl;
        return EXIT_FAILURE;
    }

    if (optFwrt) {
        cout << "Copying release to server..." << endl;

        if (!runScript(SCRIPT_CMD_RELEASE)) {
            cerr << termcolor::red << "Abort!" << termcolor::reset << endl;
            return EXIT_FAILURE;
        }
    }

	return EXIT_SUCCESS;
}
