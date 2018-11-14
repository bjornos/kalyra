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

string& firmwareRelease::getEnv()
{
    return environment;
}


vector<std::unique_ptr<packageRecipe>>& firmwareRelease::getRecipes()
{
    return recipes;
}

unique_ptr<releaseComponent>& firmwareRelease::getReleaseComponents()
{
    return releaseComponents;
}

string firmwareRelease::getReleasePrefix()
{
    return name + "_" + release + "_" + stage + build;
}

string firmwareRelease::getReleasePath()
{
    return releasePath + PLT_SLASH + name + PLT_SLASH + release + PLT_SLASH +\
         stage + PLT_SLASH + getReleasePrefix();
}

firmwareRelease::firmwareRelease(string name, string release, string stage, string build, string path,
	string env, std::vector<std::unique_ptr<packageRecipe>> recipes,
	std::unique_ptr<releaseComponent> components) :
    name(name),
    release(release),
    stage(stage),
    build(build),
    releasePath(path),
    environment(env),
    recipes(move(recipes)),
    releaseComponents(move(components))
{
}
