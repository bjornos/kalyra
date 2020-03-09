#pragma once

#include <iostream>
#include <vector>

#include "repository.hh"
#include "prod_conf.hh"


class release {
public:
    release() {};
	~release() {};

    std::string name;
	std::string stage;
	std::string build;
	std::string version;


    std::vector<repository> meta; // meta-xxxx product-embedded
	std::vector<std::string> products;
//	std::vector<product> products; // manifestet innehåller inte detta egentligen

};
