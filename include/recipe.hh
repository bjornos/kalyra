#pragma once

#include <iostream>
#include <vector>

#include "repository.hh"

class recipe {
public:
    recipe() {};
	~recipe() {};

    std::string name;
	std::string url;
	std::string revision;
	std::string source;
	std::string root;
	std::string license;
	std::string target;
	std::vector<std::string> dependency;
	std::vector<std::string> cmd_list;
};
