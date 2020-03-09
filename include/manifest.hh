#pragma once


#include "release.hh"

#include <iostream>
#include <vector>

#include <nlohmann/json.hpp>

#include "repository.hh"
#include "prod_conf.hh"
#include "recipe.hh"

constexpr auto JSON_CONF_NAME = "config"; // bogus name

constexpr auto JSON_REPO_NAME =    "name";
constexpr auto JSON_REPO_URL =     "url";
constexpr auto JSON_REPO_VERSION = "revision";


constexpr auto  JSON_SRC_NAME =    "name";
constexpr auto  JSON_SRC_URL  =    "url";
constexpr auto  JSON_SRC_VERSION = "revision";

constexpr auto JSON_PKG_RECIPE =  "recipe";
constexpr auto  JSON_PKG_TARGET =  "target";
constexpr auto JSON_PKG_REVISION = "revision";


class manifest
{
public:
    manifest() {};
    ~manifest() {};

    static std::string get_header_item(const nlohmann::json& manifest, const std::string item);

    static std::vector<repository> release_get_meta(const nlohmann::json& manifest);
    static std::vector<std::string> release_get_products(const nlohmann::json& manifest);

    static std::vector<repository> product_get_recipes(const nlohmann::json& manifest);
    static std::vector<package> product_get_packages(const nlohmann::json& manifest);
    static std::vector<std::string> product_get_cmd(const nlohmann::json& manifest, const std::string& cmd_type);
    static std::vector<std::string> product_get_artifacts(const nlohmann::json& manifest, const std::string& cmd_type);
    static std::vector<artifact> product_get_artifacts(const nlohmann::json& manifest, const std::vector<package>& packages);

    static std::unique_ptr<recipe> parse_recipe(const nlohmann::json& recipe_file);
    static std::vector<std::string> parse_recipe_target(const nlohmann::json& recipe_file, const std::string& target);

};