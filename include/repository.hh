#pragma once
#ifndef BJORNOS
#define BJORNOS

#include <iostream>

class repository
{
public:
    repository(std::string id, std::string url, std::string ver) :
        name(id),
	    url(url),
	    rev(ver)
    {
    };
    ~repository() {};

    std::string get_name() { return name; };
    std::string get_url()  { return url; };
    std::string get_rev()  { return rev; };

private:
    std::string name;
    std::string url;
    std::string rev;

};
#endif