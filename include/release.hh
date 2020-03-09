#pragma once

#include <nlohmann/json.hpp>

#include <iostream>
#include <vector>

#include "repository.hh"
#include "prod_conf.hh"


class release {
public:
    release() {};
	~release() {};

    bool load_header(const nlohmann::json& manifest);

    
    const std::string get_name() noexcept;

    //bool fetch_packages();

    std::string name;
	std::string stage;
	std::string build;
	std::string version;


    std::vector<repository> meta; // meta-xxxx product-embedded
	std::vector<std::string> products;
//	std::vector<product> products; // manifestet innehåller inte detta egentligen

};
