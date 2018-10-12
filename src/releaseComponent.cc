#include "kalyra.hh"
#include "releaseComponent.hh"

using namespace std;

releaseComponent::releaseComponent(vector<string> preCommands,
    vector<string> postCommands, vector<string> components) :
    preCommands(preCommands),
    postCommands(postCommands),
    components(components)
{

}

releaseComponent::~releaseComponent()
{

}

string releaseComponent::getFileName(string& pathName)
{
    string s = string(pathName);
    size_t pos = 0;
    const string delim = PLT_SLASH;

    // strip away everything from path name until the final slash
    while ((pos = s.find(delim)) != string::npos) {
         s.substr(0, pos);
        s.erase(0, pos + delim.length());
    }

    return s;
}