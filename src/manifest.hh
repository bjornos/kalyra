#pragma once

#include <sstream>
#include <fstream>
#include <cstdlib>
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
	static void loadTargets(std::unique_ptr<firmwareRelease>& fwrt, const cJSON* manifest);
	static void loadComponents(std::unique_ptr<firmwareRelease>& fwrt, const cJSON* manifest);
};