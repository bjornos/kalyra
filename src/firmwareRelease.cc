#include "firmwareRelease.hh"
#include "kalyra.hh"

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

string firmwareRelease::getReleasePrefix()
{
    return name + "_" + release + "_" + stage + build;
}

string firmwareRelease::getReleasePath()
{
/*         PLT_SLASH + fwrt->getName() + PLT_SLASH + fwrt->getRelease() \
             + PLT_SLASH + fwrt->getStage() + PLT_SLASH + fwrt->getReleasePrefix(); */


    return releasePath + PLT_SLASH + name + PLT_SLASH + release + PLT_SLASH +\
         stage + PLT_SLASH + getReleasePrefix();
}

firmwareRelease::firmwareRelease(string name, string release, string stage, string build, string path) :
    name(name),
    release(release),
    stage(stage),
    build(build),
    releasePath(path)
{
}
