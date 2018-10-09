#include <iostream>
#include <fstream>  

#include "scriptGenerator.hh"

using namespace std;

#if defined(_WIN32) || defined(_WIN64)
static constexpr auto isWindows = true;
#else
static constexpr auto isWindows = false;
#endif

scriptGenerator::scriptGenerator()
{

}

scriptGenerator::~scriptGenerator()
{

}

void scriptGenerator::fetch(unique_ptr<firmwareRelease>& release)
{
    std::ofstream script(SCRIPT_FETCH, std::ios_base::binary | std::ios_base::out);
    string wat = "";

    if (isWindows) {
        wat = "@";
        script << wat << "if not exist " << BUILDDIR << " md " BUILDDIR << endl;
    } else {
        script << "#!/bin/sh" << std::endl;
        script << "mkdir -p " << BUILDDIR <<"\n";
    }

    script << wat << "cd " << BUILDDIR << endl;

    for (auto& entry : release->recipes) {
        script << wat << "echo  ---- Fetching " << entry->getName() << " revision=";
        if (!entry->getRev().empty())
            script << entry->getRev();
        else
            script << "master";
        script << std::endl;

        script << wat << "git clone " << entry->getUrl() << " -q";
        if (!entry->getRev().empty())
            script << " -b " << entry->getRev();
        script << std::endl;
    }

    script.close();
}

void scriptGenerator::build(unique_ptr<firmwareRelease>& release)
{
    std::ofstream script(SCRIPT_BUILD, std::ios_base::binary | std::ios_base::out);
    string wat = "";

    if (isWindows) {
        wat = "@";
        script << wat << "if not exist " << BUILDDIR << " md " BUILDDIR << endl;
    } else {
        script << "#!/bin/sh" << std::endl;
        script << "mkdir -p " << BUILDDIR <<"\n";
    }

    script << wat << "cd " << BUILDDIR << endl;

    for (auto& entry : release->recipes) {
        script << wat << "echo build " << entry->getName() << std::endl;
        script << wat << "cd " << entry->getRoot() << std::endl;
        for (auto& t : entry->getCmdList())
            script << wat << t << " || exit 4" << endl;
        script << wat << "cd .." << std::endl;
    }

    script.close();
}

void scriptGenerator::release(unique_ptr<firmwareRelease>& release)
{
    std::ofstream script(SCRIPT_RELEASE, std::ios_base::binary | std::ios_base::out);
    string wat = "";

    if (isWindows)
        wat = "@";
    else
        script << "#!/bin/sh" << std::endl;

    script << wat << "cd " << BUILDDIR << endl;

    for (auto c : release->releaseComponents->preCommands){
        script << wat << c << std::endl;
    }

    for (auto& file : release->releaseComponents->components)
            script << wat << "cp -v " << file << " " << release->releaseComponents->releasePath << endl;

    for (auto cmd : release->releaseComponents->postCommands)
        script << wat << cmd << std::endl;

    script.close();
}