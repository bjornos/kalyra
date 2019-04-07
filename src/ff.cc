#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>


#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <filesystem>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "cJSON/cJSON.h"
#include "termcolor/termcolor.hpp"
#include "inputParser.hh"

#include "manifest.hh"
#include "packageRecipe.hh"
#include "releaseComponent.hh"
#include "firmwareRelease.hh"
#include "scriptGenerator.hh"
#include "kalyra.hh"

using namespace std;

#if defined(_WIN32) || defined(_WIN64)
// this is so extra silly
#define cerr cout
#endif

constexpr auto TAG_PRODUCT = "product";
constexpr auto TAG_RELEASE = "release";
constexpr auto TAG_STAGE = "stage";
constexpr auto TAG_BUILD = "build";
constexpr auto TAG_PATH = "release-path";
constexpr auto TAG_ENVIRONMENT = "environment";
constexpr auto TAG_PACKAGES = "packages";

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
    unique_ptr<InputParser> options(new InputParser(argc, argv));

    cout <<  termcolor::cyan << KALYRA_BANNER << " v" << KALYRA_MAJOR << "." << KALYRA_MINOR << "." << KALYRA_SUB << termcolor::reset << endl;

    cout << "Firmware Factory: ";

    if (options->showHelp()){
    	cout << "Available command options:" << endl;
        cout << "-m, --manifest <name>   : Project manifest file (mandatory)." << endl;
        cout << "-f, --fetch <recipe>    : Fetch recipe repository only. No argument means all recipes." << endl;
        cout << "-b, " << OPT_BUILD_LONG << " <recipe>    : Build recipe only. If no recipe is stated, all recipes are built." << endl;
        cout << OPT_UPDATE_SHORT << ", " << OPT_UPDATE_LONG << " <recipe>   : " << OPT_UPDATE_DESC << endl;
        cout << "-c, --clean             : Clean working directory" << endl;
        cout << "-r, --recipes           : Show avaiable recipes for current manifest" << endl;
        cout << "-g, --generate          : Generate build scripts only" << endl;
        cout << "--fwrt                  : Firmware Release Tool. Generate a official release after building all components." << endl;
        cout << "--yes,                  : Don't stop and wait for user input, assume yes on all." << endl;
    	return EXIT_SUCCESS;
    }

    if (options->clean()) {
        cout << "Clean up workspace..." << endl;

        if (!options->alwaysYes()) {
            char answer;
            cout << "This will erase everything laying around in the " BUILDDIR " directory." << endl;
            cout << "OK to proceed (y/n)? ";
            cin >> answer;
            if ((answer != 'y') && (answer != 'Y')) {
                cout << "Nothing removed." << endl;
                return EXIT_SUCCESS;
            }
        }

#if defined(_WIN32) || defined(_WIN64)
        return std::system("rm -rf " BUILDDIR " .kalyra-manifest");
#else
        return (std::filesystem::remove_all(BUILDDIR) || std::filesystem::remove(".kalyra-manifest"));
#endif
    }

    if (options->getManifest().empty()) {
        cerr <<  "Missing manifest file. Try --help" <<  endl;
        return EXIT_FAILURE;
    }

    cout << "Processing " << options->getManifest() << "... " << endl << endl;

    try {
        manifest::loadHeader(manifest, options->getManifest());
    } catch (const exception& e) {
        cerr << termcolor::red <<e.what() << termcolor::reset << endl <<  "Abort!" << endl;
        // No valid manifest is set
        remove(".kalyra-manifest");
        return EXIT_FAILURE;
    }

    const auto product = manifest::getValue(manifest, TAG_PRODUCT);
    const auto release = manifest::getValue(manifest, TAG_RELEASE);
    const auto stage = manifest::getValue(manifest, TAG_STAGE);
    const auto build = manifest::getValue(manifest, TAG_BUILD);
    const auto rPath = manifest::getValue(manifest, TAG_PATH);
    const auto env = manifest::getValue(manifest, TAG_ENVIRONMENT);

    vector<unique_ptr<packageRecipe>> recipes;
    try {
        recipes = manifest::loadRecipes(manifest);
    } catch (const exception& e) {
        cerr << termcolor::red <<e.what() << termcolor::reset << endl <<  "Abort!" << endl;
        return EXIT_FAILURE;
    }

    for (auto& entry : recipes) {
        try {
            ifstream r(RECIPE_DIRECTORY + entry->getName() + RECIPE_SUFFIX);
            packageRecipe::parseRecipe(r, entry);
        } catch (const exception& e) {
            cerr << termcolor::red << e.what() << termcolor::reset << endl << "Abort!" << endl;
            return EXIT_FAILURE;
        }
    }

    if (options->showRecipes()) {
        cout << termcolor::yellow << "Available recipes:" << termcolor::reset << endl;
        for (auto& entry : recipes)
            cout << entry->getName() << endl;
        return EXIT_SUCCESS;
    }

    auto components(unique_ptr<releaseComponent>(manifest::loadComponents(recipes, manifest)));

    auto fwrt(unique_ptr<firmwareRelease>(new firmwareRelease(product->valuestring,
        release->valuestring, stage->valuestring, build->valuestring, rPath->valuestring,
        env->valuestring, move(recipes), move(components))));


    if (options->buildOnly() && (fwrt->hasRecipe(options->getBuildSingle()) == false)) {
        cerr << termcolor::red << "Could not find any recipe for '" << options->getBuildSingle() << "'" << termcolor::reset << endl;
        return EXIT_FAILURE;
    }

   if (options->FWRT()) {
        cout << "Release: " << termcolor::yellow << fwrt->getName() << " " << fwrt->getRelease() \
            << " " << fwrt->getStage() <<  fwrt->getBuild() << termcolor::reset << endl;

        cout << "Release Path: " << termcolor::yellow << fwrt->getReleasePath() << \
            termcolor::reset << endl << endl;

        if (!options->alwaysYes()) {
            cout << "Proceed? (Y/N): ";
            char answer;
            cin >> answer;
            cout << "answer: " << answer << endl;
            if ((answer != 'y') && (answer != 'Y')) {
                cout << endl << "Bailing out." << endl;
                return EXIT_SUCCESS;
            }
        }
        scriptGenerator::release(fwrt, options->getManifest());
    }

    if (options->generateOnly())
        cout << "Generating build scripts...." << endl;


#if defined(_WIN32) || defined(_WIN64)
    if (!CreateDirectory(BUILDDIR, NULL) && (GetLastError() != ERROR_ALREADY_EXISTS)) {
        return EXIT_FAILURE;
    }
#else
    if (!std::filesystem::exists(BUILDDIR) && !std::filesystem::create_directory(BUILDDIR)) {
        cerr << termcolor::red << "Failed to create build directory." << termcolor::reset << endl;
        return EXIT_FAILURE;
    }
#endif

    scriptGenerator::fetch(fwrt, options->getFetchSingle(), options->updateOnly(), options->getUpdateSingle());

    scriptGenerator::build(fwrt, options->getBuildSingle());

    if (options->generateOnly())
        return EXIT_SUCCESS;

    if (options->buildOnly() == false) {
        if (!runScript(SCRIPT_CMD_FETCH)) {
            cerr << termcolor::red << "Error fetching target" << termcolor::reset << endl;
            return EXIT_FAILURE;
        }

        if (options->fetchOnly())
            return EXIT_SUCCESS;
    }

    if (!runScript(SCRIPT_CMD_BUILD)) {
        cerr << termcolor::red << "Abort!" << termcolor::reset << endl;
        return EXIT_FAILURE;
    }

    if (options->FWRT()) {
        cout << "Copying release to server..." << endl;

        if (!runScript(SCRIPT_CMD_RELEASE)) {
            cerr << termcolor::red << "Firmware release tool failed!" << termcolor::reset << endl;
            auto rmRelDir("rm -rf " + fwrt->getReleasePath());
            std::system(rmRelDir.c_str());
            return EXIT_FAILURE;
        }

        if (!options->alwaysYes()) {
            cout << "Files have been copied. Proceed and set git release tag? (Y/N): ";
            char answer;
            cin >> answer;

            if ((answer != 'y') && (answer != 'Y')) {
                cout << endl << "Skip git tag." << endl;
                return EXIT_SUCCESS;
            }
        }

        scriptGenerator::gitTag(fwrt);

        if (!runScript(SCRIPT_CMD_GITTAG)) {
            cerr << termcolor::red << "Failed to set git tag! Release abort." << termcolor::reset << endl;
#if defined(_WIN32) || defined(_WIN64)
            auto rmRelDir("rm -rf " + fwrt->getReleasePath());
            std::system(rmRelDir.c_str());
#else
            std::filesystem::remove_all(fwrt->getReleasePath());
#endif
            return EXIT_FAILURE;
        }
    }

	return EXIT_SUCCESS;
}
