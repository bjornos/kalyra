#include "termcolor/termcolor.hpp"

#include "kalyra.hh"
#include "release.hh"
#include "manifest.hh"

using namespace std;
using json = nlohmann::json;



const std::string release::get_name() noexcept
{
    return this->name;
}


bool release::load_header(const json& manifest)
{
    auto retval = true;

    this->name = manifest::get_header_item(manifest, "name");
    if (this->name.empty()) {
        throw logic_error("name");
    }
    this->version = manifest::get_header_item(manifest, "version");
    this->stage = manifest::get_header_item(manifest, "stage");
    this->build = manifest::get_header_item(manifest, "build");
    if (this->build.empty()) {
        throw logic_error("build");
    }



    return retval;
}