#pragma once

#include <iostream>
#include <memory>
#include <vector>

class packageRecipe {
public:
	packageRecipe(std::string name, std::string revisionOverride, std::string targetOverride);
	~packageRecipe();

    static void parseRecipe(std::unique_ptr<packageRecipe>& recipe);

    std::string& getName();
    std::string& getRev();
    std::string& getUrl();
    std::string& getRoot();
    std::string& getTarget();
    std::vector<std::string>& getCmdList();

    std::string source;
    std::string sha;
    std::string depends;
    std::string license;

private:
    std::string name;
    std::string revisionOverride;
    std::string revision;
    std::string url;
    std::string root;
    std::string targetOverride;
    std::string target;
    std::vector<std::string> commandList;
};
