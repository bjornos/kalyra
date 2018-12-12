#pragma once

#include <vector>
#include <memory>

#include "packageRecipe.hh"
#include "releaseComponent.hh"

class firmwareRelease { 
public:
    firmwareRelease(std::string name, std::string release, std::string stage,
        std::string build, std::string path, std::string env, std::vector<std::unique_ptr<packageRecipe>> recipes,
        std::unique_ptr<releaseComponent> components);
    ~firmwareRelease() {};

    std::string& getName();
    std::string& getRelease();
    std::string& getStage();
    std::string& getBuild();
    std::string& getEnv();
    bool hasRecipe(std::string& recipe);

    std::vector<std::unique_ptr<packageRecipe>>& getRecipes();
    std::unique_ptr<releaseComponent>& getReleaseComponents();

    std::string getReleasePrefix();
    std::string getReleasePath();

private:
    std::string name;
    std::string release;
    std::string stage;
    std::string build;
    std::string releasePath;
    std::string environment;
    std::vector<std::unique_ptr<packageRecipe>> recipes;
    std::unique_ptr<releaseComponent> releaseComponents;
};
