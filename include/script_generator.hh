#pragma once

#include <vector>
#include <iostream>
#include <memory>

#include "input_parser.hh"
#include "repository.hh"
#include "recipe.hh"
#include "prod_conf.hh"

class script_generator {
public:
    script_generator() {};
    ~script_generator() {};

    static bool run_script(const std::string& cmd);

    static void fetch(std::vector<repository>& repos, const std::string& script_file, const std::string& path, const std::unique_ptr<InputParser>& options);

//static void build(const std::unique_ptr<recipe>& recipe, const std::string& script_file, const std::string& path);
static void build(const std::vector<std::unique_ptr<recipe>>& recipes, const std::string& script_file, const std::string& path, const std::unique_ptr<InputParser>& options);
static void release(const std::unique_ptr<product>& prod, const std::string& script_file, const std::string& path);



/*    static void fetch(std::unique_ptr<firmwareRelease>& release, const std::string& singleTarget, bool update, const std::string& updateTarget>
    static void build(std::unique_ptr<firmwareRelease>& release, const std::string& singleTarget);
    static void release(std::unique_ptr<firmwareRelease>& release, const std::string& manifest);
    static void gitTag(std::unique_ptr<firmwareRelease>& release); */
};

