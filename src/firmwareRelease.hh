#pragma once

#include <vector>
#include <memory>

#include "packageRecipe.hh"

class firmwareRelease { 
public:
    firmwareRelease(std::string name, std::string release, std::string stage, std::string build);
    ~firmwareRelease() {};

    std::string& getName();
    std::string& getRelease();
    std::string& getStage();
    std::string& getBuild();

    std::vector<std::unique_ptr<packageRecipe>> recipes;

private:
    std::string name;
    std::string release;
    std::string stage;
    std::string build;
};
