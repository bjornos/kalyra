#include <sstream>
#include <fstream>
#include <cstdlib>

#include "packageRecipe.hh"
#include "cJSON/cJSON.h"

using namespace std;

constexpr auto RECIPE_DIRECTORY = "recipes/";
constexpr auto RECIPE_SUFFIX = ".rp";

packageRecipe::packageRecipe(string name, string revisionOverride, string targetOverride) :
    name(name),
    revisionOverride(revisionOverride),
    targetOverride(targetOverride)
{
}

packageRecipe::packageRecipe(string name, string url) :
    name(name),
    url(url)
{
}

packageRecipe::~packageRecipe()
{
}

void packageRecipe::parseRecipe(unique_ptr<packageRecipe>& recipe)
{
    std::ifstream r(RECIPE_DIRECTORY + recipe->name + RECIPE_SUFFIX);
    std::stringstream buffer;

    buffer << r.rdbuf();

    auto buildPackage = cJSON_Parse(buffer.str().c_str());
    if (buildPackage == NULL) {
        cerr << cJSON_Print(buildPackage);
        const auto errPtr = cJSON_GetErrorPtr();
        if (errPtr == buffer.str()) {
            cJSON_Delete(buildPackage);
            throw std::invalid_argument("Recipe for " + recipe->name + " not found");
        } else {
            cJSON_Delete(buildPackage);
            cout << errPtr << endl;
            const string error(recipe->name + ": Parse error");
            throw std::invalid_argument(error);
        }
    }

    const auto packageProp = cJSON_GetObjectItemCaseSensitive(buildPackage, "package");

    auto name = cJSON_GetObjectItemCaseSensitive(packageProp, "name");
    if ((name != NULL) && (cJSON_IsString(name))) {
        recipe->name = name->valuestring;
    }

    auto url = cJSON_GetObjectItemCaseSensitive(packageProp, "url");
    if ((url != NULL) && (cJSON_IsString(url))) {
            recipe->url = url->valuestring;
    }

    auto root = cJSON_GetObjectItemCaseSensitive(packageProp, "root");
    if ((root != NULL) && (cJSON_IsString(root))) {
            recipe->root = root->valuestring;
    }

    auto src = cJSON_GetObjectItemCaseSensitive(packageProp, "source");
    if ((src != NULL) && (cJSON_IsString(src))) {
        recipe->source = src->valuestring;
    }

    if (recipe->revisionOverride.empty()) {
        auto revision = cJSON_GetObjectItemCaseSensitive(packageProp, "revision");
        if ((revision != NULL) && (cJSON_IsString(revision))) {
            recipe->revision = revision->valuestring;
        }
    } else {
    	recipe->revision = recipe->revisionOverride;
    }

    auto license = cJSON_GetObjectItemCaseSensitive(packageProp, "license");
    if ((license != NULL) && (cJSON_IsString(license))) {
        recipe->license = license->valuestring;
    }

    string realTarget;
    if (recipe->targetOverride.empty()) {
        auto target = cJSON_GetObjectItemCaseSensitive(packageProp, "target");
        realTarget = target->valuestring;
    }
    else 
    	realTarget = recipe->targetOverride;

    auto cmdList = cJSON_GetObjectItemCaseSensitive(buildPackage, realTarget.c_str());
    if (cmdList == NULL) {
        auto errMsg(recipe->name + ": No target found.");
        throw std::invalid_argument(errMsg);
    }

    cJSON* cmd;
    cJSON_ArrayForEach(cmd, cmdList)
    {
        recipe->commandList.emplace_back(cmd->valuestring);
    }

    cJSON_Delete(buildPackage);
}

string& packageRecipe::getName()
{
    return name;
}

string& packageRecipe::getRev()
{
    return revision;
}

string& packageRecipe::getTarget()
{
    return target;
}

string& packageRecipe::getUrl()
{
    return url;
}

string& packageRecipe::getRoot()
{
    return root;
}
std::vector<std::string>& packageRecipe::getCmdList()
{
    return commandList;
}
