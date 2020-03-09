#pragma once

#include <iostream>
#include <vector>

#include "repository.hh"
#include "package.hh"

class artifact
{
public:
    artifact() {};
    ~artifact() {};

    std::string target;
    std::vector<std::tuple<std::string, std::string>> item;
};

class product {
public:
    product() {};
    ~product() {};

    //static void prod_config();

    std::string name;
    std::string version;
	std::vector<repository> recipes;
	std::vector<package> packages;

    std::vector<std::string> cmd_pre;
    //std::vector<std::tuple<std::string, std::string>> artifacts;
    std::vector<artifact> artifacts;
    std::vector<std::string> cmd_post;


};