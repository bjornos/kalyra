#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "cJSON/cJSON.h"
#include "termcolor/termcolor.hpp"
#include "packageRecipe.hh"
#include "inputParser.hh"
#include "kalyra.hh"

using namespace std;

constexpr auto DEFAULT_RECIPE = "recipe-generated.rp";
constexpr auto DEFAULT_RECIPE_PATH = "recipes";

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

void createRecipe(unique_ptr<packageRecipe>& r, string& path)
{
    cJSON* recipe = cJSON_CreateObject();
    cJSON* package = cJSON_CreateObject();


    cJSON_AddItemToObject(recipe, "package", package);

    if (cJSON_AddStringToObject(package, "name", r->getName().c_str()) == NULL) {
        throw std::logic_error("Failed to create name section");
    }
    if (cJSON_AddStringToObject(package, "source", "") == NULL) {
        throw std::logic_error("Failed to create source section");
    }
    if (cJSON_AddStringToObject(package, "url", r->getUrl().c_str()) == NULL) {
        throw std::logic_error("Failed to create url section");
    }
    if (cJSON_AddStringToObject(package, "root", "") == NULL) {
        throw std::logic_error("Failed to create root section");
    }
    if (cJSON_AddStringToObject(package, "license", "") == NULL) {
        throw std::logic_error("Failed to create license section");
    }
    if (cJSON_AddStringToObject(package, "revision", "") == NULL) {
        throw std::logic_error("Failed to create revision section");
    }
    if (cJSON_AddStringToObject(package, "target", "") == NULL) {
        throw std::logic_error("Failed to create target section");
    }
    if (cJSON_AddStringToObject(package, "depends", "") == NULL) {
        throw std::logic_error("Failed to create depends section");
    }
    if (cJSON_AddStringToObject(package, "environment", "") == NULL) {
        throw std::logic_error("Failed to create environment section");
    }

    cJSON* target = cJSON_AddArrayToObject(recipe, "default-target");
    if (target == NULL) {
        throw std::logic_error("default-target section");
    }

    auto targ1 = cJSON_CreateString("command 1");
    cJSON_AddItemToArray(target, targ1);

    auto targ2 = cJSON_CreateString("command 2");
    cJSON_AddItemToArray(target, targ2);

#if defined(_WIN32) || defined(_WIN64)
    auto delimiter = "\\";
#else
    auto delimiter = "/";
#endif

   std::ofstream outRecipe(path + delimiter + r->getName() + ".rp", std::ios_base::binary | std::ios_base::out);

   outRecipe << cJSON_Print(recipe) << endl;
   outRecipe.close();

   cJSON_Delete(recipe);
}

int main(int argc, char *argv[])
{
    InputParser cmdOptions(argc, argv);
    string recipeName;
    string outPath;

    cout <<  termcolor::cyan << KALYRA_BANNER << " v" << KALYRA_MAJOR << "." << KALYRA_MINOR << "." << KALYRA_SUB << termcolor::reset << endl;

    cout <<  "Recipe Generator: ";

    if (cmdOptions.cmdOptionExists("--help")){
        cout << "Available command options:" << endl;
        cout << "-u, --url <url>     : URL or path to repository." << endl;
        cout << "-n, --name <name>   : Recipe name. If omitted name will simply be "<< DEFAULT_RECIPE << endl;
        cout << "-p, --path <path>   : Path where recipe shall be generated. Default is '" << DEFAULT_RECIPE_PATH << "'" << endl;
        return EXIT_SUCCESS;
    }

    auto url(cmdOptions.getCmdOption("-u"));
    if (url.empty()) {
        url.assign (cmdOptions.getCmdOption("--url"));
        if (url.empty()) {
            cerr <<  "Missing URL/Path. Try --help" <<  endl;
            return EXIT_FAILURE;
        }
    }

    recipeName = cmdOptions.getCmdOption("-n");
    if (recipeName.empty()) {
        recipeName = cmdOptions.getCmdOption("--name");
        if (recipeName.empty())
            recipeName.assign(DEFAULT_RECIPE);
    }

    outPath = cmdOptions.getCmdOption("-p");
    if (outPath.empty()) {
        outPath = cmdOptions.getCmdOption("--path");
        if (outPath.empty())
        	outPath.assign(DEFAULT_RECIPE_PATH);
    }

    cout << "Generating recipe..." << endl << endl;

    auto rp(unique_ptr<packageRecipe>(new packageRecipe(recipeName, url)));

    cout << "Recipe: " << termcolor::yellow << recipeName << termcolor::reset << endl;
    cout << "URL: " << termcolor::yellow << url << termcolor::reset << endl << endl;

    createDir(outPath);

    try {
        createRecipe(rp, outPath);
    } catch (std::exception& e) {
        cerr << e.what() << " Abort!" << endl;
        return EXIT_FAILURE;
    }

	return EXIT_SUCCESS;
}
