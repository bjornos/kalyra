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

const string gitClone(std::unique_ptr<packageRecipe> &entry)
{
    string clone("git clone " + entry->getUrl() + " -q");

    if (!entry->getRev().empty()) {
        clone.append(" -b ");
        clone.append(entry->getRev());
    }

    return clone;
}

void scriptGenerator::fetch(unique_ptr<firmwareRelease>& release, const string& singleTarget)
{
    std::ofstream script(SCRIPT_FETCH, std::ios_base::binary | std::ios_base::out);
    string comment = "# ";
    bool targetFetch = false;

    if (isWindows) {
        comment.assign("REM ");
        script << "@echo off" << endl;
        script << "IF NOT EXIST " << BUILDDIR << " md " << BUILDDIR << endl;
    } else {
        script << "#!/bin/sh" << std::endl;
        script << "mkdir -p " << BUILDDIR <<"\n";
    }

    script << "cd " << BUILDDIR << " || exit 1"<< endl;

    for (auto& entry : release->getRecipes()) {

        if (singleTarget.empty()) {
            // The normal case - fetch all recipe components in manifest
            script << "echo  ---- Fetching " << entry->getName() << " revision=";
            if (!entry->getRev().empty())
                script << entry->getRev();
            else
                script << "master";
            script << std::endl;

            if (isWindows) {
                script << "IF NOT EXIST " << entry->getRoot() << " (" << gitClone(entry) << " || exit 1)";
                script << " ELSE (@echo  **** Using local mirror)" << endl;
            } else {
                script << "if [ -f " << entry->getRoot() << " ]; then" << endl;
                script << "echo  \" **** Using local mirror\"" << endl;
                script << "else"  << endl;
                script << gitClone(entry)  << "|| exit 1" << endl << "fi" << endl;
            }

            //script << "touch " << BUILDDIR << PLT_SLASH << "." << entry->getRoot() << "-fetched" << endl;
            targetFetch = true;
        } else if (singleTarget.compare(entry->getName()) == 0) {
            // Fetch only one recipe component
            script << "echo  ---- Fetching " << entry->getName() << " revision=";
            if (!entry->getRev().empty())
                script << entry->getRev();
            else
                script << "master";
            script << " || exit 1 " << std::endl;


            if (isWindows) {
                script << "IF EXIST " << entry->getRoot() << " rm -rf " << entry->getRoot() << endl;
            } else {
                script << "rm -rf " << entry->getRoot() << endl;
            }
            script << gitClone(entry) << " || exit 1" << endl;
            //script << "touch " << BUILDDIR << PLT_SLASH << "." << entry->getRoot() << "-fetched" << endl;
            targetFetch = true;
        }
    }

    if (!targetFetch)
        script << comment << "No targets found. Thats an error." << endl << "exit 1" << endl;

    script.close();
}

void scriptGenerator::build(unique_ptr<firmwareRelease>& release, const string& singleTarget)
{
    std::ofstream script(SCRIPT_BUILD, std::ios_base::binary | std::ios_base::out);

    if (isWindows) {
        script << "@echo off" << endl;
        script << "IF NOT EXIST " << BUILDDIR << " md " << BUILDDIR << endl;
    } else {
        script << "#!/bin/sh" << std::endl;
        script << "mkdir -p " << BUILDDIR <<"\n";
    }

    script << "cd " << BUILDDIR << " || exit 1" << endl;

    for (auto& entry : release->getRecipes()) {
        if (singleTarget.empty() || (entry->getName().compare(singleTarget) == 0)) {
            script << "echo build " << entry->getName() << std::endl;
            script << "cd " << entry->getRoot() << " || exit 1" << std::endl;
            for (auto& t : entry->getCmdList()) {
            if (!t.empty())
                script << t << " || exit 1" << endl;
            }
            script << "cd .." << std::endl;
        }
    }

    script.close();
}

void scriptGenerator::release(unique_ptr<firmwareRelease>& release)
{
    std::ofstream script(SCRIPT_RELEASE, std::ios_base::binary | std::ios_base::out);

    if (isWindows) {
        script << "@echo off" << endl;
        script << "IF NOT EXIST " << release->getReleasePath() << " md " << release->getReleasePath() << endl;
    } else {
        script << "#!/bin/sh" << std::endl;
        script << "mkdir -p " << release->getReleasePath() << " || exit 1" << endl;
    }

    script << "cd " << BUILDDIR << " || exit 1" << endl;

    for (auto c : (release->getReleaseComponents())->getPreCommands()) {
        script << c << " || exit 1" << std::endl;
    }

    for (auto& file : (release->getReleaseComponents())->getComponents())
            script << "cp -v " << file << " " << \
            release->getReleasePath() + PLT_SLASH + release->getReleasePrefix() << "_" << (release->getReleaseComponents())->getFileName(file) \
            << " || exit 1" << endl;

    for (auto cmd : (release->getReleaseComponents())->getPostCommands())
        script << cmd << " || exit 1" << std::endl;

    script.close();
}