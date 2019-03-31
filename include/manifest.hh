#pragma once

#include <sstream>
#include <fstream>
#include <cstdlib>
#include <vector>

#include "cJSON/cJSON.h"
#include "termcolor/termcolor.hpp"

#include "firmwareRelease.hh"
#include "packageRecipe.hh"
#include "releaseComponent.hh"


class manifest {
public:
    manifest() {};
    ~manifest() {};
    
    static const cJSON* getValue(const cJSON* recipe, std::string tag);
	static void loadHeader(const cJSON*& m, const std::string& manifest);
    static std::vector<std::unique_ptr<packageRecipe>> loadRecipes(const cJSON* manifest);
	static std::unique_ptr<releaseComponent> loadComponents(std::vector<std::unique_ptr<packageRecipe>>& recipes, const cJSON* manifest);
};