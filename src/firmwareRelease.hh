#pragma once

#include <vector>
#include <memory>

#include "packageRecipe.hh"
#include "releaseComponent.hh"

class firmwareRelease { 
public:
    firmwareRelease(std::string name, std::string release, std::string stage, std::string build);
    ~firmwareRelease() {};

    std::string& getName();
    std::string& getRelease();
    std::string& getStage();
    std::string& getBuild();

    std::vector<std::unique_ptr<packageRecipe>> recipes;
    std::unique_ptr<releaseComponent> releaseComponents;

private:
    std::string name;
    std::string release;
    std::string stage;
    std::string build;
};
