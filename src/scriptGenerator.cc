#include <iostream>
#include <fstream>  

#include "scriptGenerator.hh"

using namespace std;

constexpr auto HOMEDIR = "HOME";


scriptGenerator::scriptGenerator()
{

}

scriptGenerator::~scriptGenerator()
{

}

void scriptGenerator::fetch(unique_ptr<firmwareRelease>& release)
{
    std::ofstream script(SCRIPT_FETCH, std::ios_base::binary | std::ios_base::out);

    script << "#!/bin/sh" << std::endl;
    script << "HOME=$PWD" << endl;
    script << "mkdir -p " << BUILDDIR <<"\n";
    script << "cd " << BUILDDIR << endl;

    for (auto& entry : release->recipes) {
        script << "echo  ---- Fetching " << entry->getName() << " revision=";
        if (!entry->getRev().empty())
            script << entry->getRev();
        else
            script << "\\<master\\>";
        script << std::endl;

        script << "git clone " << entry->getUrl();
        if (!entry->getRev().empty())
            script << " -b " << entry->getRev();
        script << std::endl;

    }

    script.close();

}

void scriptGenerator::build(unique_ptr<firmwareRelease>& release)
{
    std::ofstream script(SCRIPT_BUILD, std::ios_base::binary | std::ios_base::out);

    script << "#!/bin/sh" << std::endl;
    script << "HOME=$PWD" << endl;
    script << "mkdir -p " << BUILDDIR <<"\n";
    script << "cd " << BUILDDIR << endl;

    for (auto& entry : release->recipes) {
        script << "echo build " << entry->getName() << std::endl;
        script << "cd " << entry->getRoot() << std::endl;
        for (auto& t : entry->getCmdList())
            script << t << endl;
        script << "cd .." << std::endl;
    }

    script.close();
}

void scriptGenerator::release(unique_ptr<firmwareRelease>& release)
{
    std::ofstream script(SCRIPT_RELEASE, std::ios_base::binary | std::ios_base::out);

    //script << "#!/bin/sh" << std::endl;
    //script << "HOME=$PWD" << endl;
    //script << "mkdir -p " << BUILDDIR <<"\n";
    script << "@cd " << BUILDDIR << endl;

    for (auto c : release->releaseComponents->preCommands){
        script << "@" << c << std::endl;
    }

    for (auto& file : release->releaseComponents->components)
            script << "@cp -v " << file << " " << release->releaseComponents->releasePath << endl;

    for (auto cmd : release->releaseComponents->postCommands)
        script << "@" << cmd << std::endl;

    script.close();
}