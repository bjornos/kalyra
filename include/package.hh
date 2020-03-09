#pragma once

#include <iostream>
#include <vector>

class package {
	public:
    package(std::string id, std::string targ, std::string v) :
        recipe(id),
	    target(targ),
	    override(v)
    {
    };
    ~package() {};

	std::string recipe;
	std::string target;
	std::string override;

};
