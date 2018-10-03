#include "firmwareRelease.hh"

using namespace std;

string& firmwareRelease::getName()
{
    return name;
}

string& firmwareRelease::getRelease()
{
    return release;
}

string& firmwareRelease::getStage()
{
    return stage;
}

string& firmwareRelease::getBuild()
{
    return build;
}

firmwareRelease::firmwareRelease(string name, string release, string stage, string build) :
    name(name),
    release(release),
    stage(stage),
    build(build)
{
}
