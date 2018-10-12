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
        script << wat << "if not exist " << BUILDDIR << " md " << BUILDDIR << endl;
    } else {
        script << "#!/bin/sh" << std::endl;
        script << "mkdir -p " << BUILDDIR <<"\n";
    }

    script << wat << "cd " << BUILDDIR << " || exit 1"<< endl;

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

void scriptGenerator::build(unique_ptr<firmwareRelease>& release, const string& singleTarget)
{
    std::ofstream script(SCRIPT_BUILD, std::ios_base::binary | std::ios_base::out);
    string wat = "";

    if (isWindows) {
        wat = "@";
        script << wat << "if not exist " << BUILDDIR << " md " << BUILDDIR << endl;
    } else {
        script << "#!/bin/sh" << std::endl;
        script << "mkdir -p " << BUILDDIR <<"\n";
    }

    script << wat << "cd " << BUILDDIR << " || exit 1" << endl;

    for (auto& entry : release->recipes) {
        if (singleTarget.empty() || (entry->getName().compare(singleTarget) == 0)) {
            script << wat << "echo build " << entry->getName() << std::endl;
            script << wat << "cd " << entry->getRoot() << " || exit 1" << std::endl;
            for (auto& t : entry->getCmdList()) {
            if (!t.empty())
                script << wat << t << " || exit 1" << endl;
            }
            script << wat << "cd .." << std::endl;
        }
    }

    script.close();
}

void scriptGenerator::release(unique_ptr<firmwareRelease>& release)
{
    std::ofstream script(SCRIPT_RELEASE, std::ios_base::binary | std::ios_base::out);
    string wat = "";

    if (isWindows) {
        wat = "@";
        script << wat << "if not exist " << release->getReleasePath() << " md " << release->getReleasePath() << endl;

    } else {
        script << "#!/bin/sh" << std::endl;
        script << "mkdir -p " << release->getReleasePath() << " || exit 1" << endl;
    }

    script << wat << "cd " << BUILDDIR << " || exit 1" << endl;

    for (auto c : release->releaseComponents->getPreCommands()){
        script << wat << c << " || exit 1" << std::endl;
    }

    for (auto& file : release->releaseComponents->getComponents())
            script << wat << "cp -v " << file << " " << \
            release->getReleasePath() + PLT_SLASH + release->getReleasePrefix() << "_" << release->releaseComponents->getFileName(file) \
            << " || exit 1" << endl;

    for (auto cmd : release->releaseComponents->getPostCommands())
        script << wat << cmd << " || exit 1" << std::endl;

    script.close();
}