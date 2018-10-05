#include "releaseComponent.hh"

using namespace std;

releaseComponent::releaseComponent(vector<string> preCommands,
    vector<string> postCommands, vector<string> components, string path) :
    preCommands(preCommands),
    postCommands(postCommands),
    components(components),
    releasePath(path)
{

}

releaseComponent::~releaseComponent()
{

}