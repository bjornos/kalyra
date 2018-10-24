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
    string wat = "";
    string comment = "# ";
    bool targetFetch = false;

    if (isWindows) {
        wat = "@";
        comment.assign("REM ");
        script << wat << "IF NOT EXIST " << BUILDDIR << " md " << BUILDDIR << endl;
    } else {
        script << "#!/bin/sh" << std::endl;
        script << "mkdir -p " << BUILDDIR <<"\n";
    }

    script << wat << "cd " << BUILDDIR << " || exit 1"<< endl;

    for (auto& entry : release->getRecipes()) {

        if (singleTarget.empty()) {
            // The normal case - fetch all recipe components in manifest
            script << wat << "echo  ---- Fetching " << entry->getName() << " revision=";
            if (!entry->getRev().empty())
                script << entry->getRev();
            else
                script << "master";
            script << std::endl;

            if (isWindows) {
                script << wat << "IF NOT EXIST ." << entry->getName() << "-fetched (" << gitClone(entry) << ")";
                script << " ELSE (@echo  **** Using local mirror)" << endl;
            } else {
                script << "if [ -ne ." << entry->getName() << "-fetched ]" << endl;
                script << gitClone(entry)  << endl;
                script << "else echo  **** Using local mirror" << endl;
            }

            script << wat << "touch ." << entry->getName() << "-fetched" << endl;
            targetFetch = true;
        } else if (singleTarget.compare(entry->getName()) == 0) {
            // Fetch only one recipe component
            script << wat << "echo  ---- Fetching " << entry->getName() << " revision=";
            if (!entry->getRev().empty())
                script << entry->getRev();
            else
                script << "master";
            script << " || exit 1 " << std::endl;


            if (isWindows) {
                script << wat << "IF EXIST " << entry->getName() << " rm -rf " << entry->getName() << endl;
            } else {
                script << "rm -rf " << entry->getName() << endl;
            }
            script << wat << gitClone(entry) << " || exit 1" << endl;
            script << wat << "touch ." << entry->getName() << "-fetched" << endl;
            targetFetch = true;
        }
    }

    if (!targetFetch)
        script << wat << comment << "No targets found. Thats an error." << endl << wat << "exit 1" << endl;

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

    for (auto& entry : release->getRecipes()) {
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

    for (auto c : (release->getReleaseComponents())->getPreCommands()) {
        script << wat << c << " || exit 1" << std::endl;
    }

    for (auto& file : (release->getReleaseComponents())->getComponents())
            script << wat << "cp -v " << file << " " << \
            release->getReleasePath() + PLT_SLASH + release->getReleasePrefix() << "_" << (release->getReleaseComponents())->getFileName(file) \
            << " || exit 1" << endl;

    for (auto cmd : (release->getReleaseComponents())->getPostCommands())
        script << wat << cmd << " || exit 1" << std::endl;

    script.close();
}