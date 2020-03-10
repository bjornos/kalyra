#pragma once

#include <nlohmann/json.hpp>

#include <iostream>
#include <vector>

#include "repository.hh"
#include "prod_conf.hh"


class release {
public:
    release(const nlohmann::json& product_manifest);
	~release() {};

    void init();

    
    const std::string get_name() noexcept;


    std::vector<std::unique_ptr<product>> get_products();


    std::string name;
	std::string stage;
	std::string build;
	std::string version;

    std::vector<repository> meta; // meta-xxxx product-embedded
	std::vector<std::string> products;

private:
    nlohmann::json manifest;
};
