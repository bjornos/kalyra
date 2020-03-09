#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <filesystem>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <nlohmann/json.hpp>

#include <list>
#include <vector>

#include "termcolor/termcolor.hpp"
#include "input_parser.hh"

using namespace std;
// for convenience
using json = nlohmann::json;




#define JSON_META_NAME     "name"
#define JSON_META_URL      "url"
#define JSON_META_VERSION  "version"

#define JSON_SRC_NAME     "name"
#define JSON_SRC_URL      "url"
#define JSON_SRC_VERSION  "version"

#define JSON_PKG_RECIPE   "recipe"
#define JSON_PKG_TARGET   "target"
#define JSON_PKG_REVISION "revision"


#define JSON_CONF_NAME     "config"

class repository {
public:
    repository(std::string id, std::string url, std::string v) :
        name(id),
	    url(url),
	    version(v)
    {
    };
    ~repository() {};

	string name;
	string url;
	string version;

};

class package // toolchain
{
	public:
    package(std::string id, std::string targ, std::string v) :
        recipe(id),
	    target(targ),
	    override(v)
    {
    };
    ~package() {};

	string recipe;
	string target;
	string override;

};

class packageRecipe
{
public:
    string name;
	string source;
	string url;
	string root;
	string license;
	string revision;
	string target;
	vector<string> dependency;
	vector<string> cmdList;
};

class productConfiguration
{
	public:
    string name;
	vector<repository> recipes;
	vector<package> packages;

    vector<string> preCmd;
    vector<tuple<string, string>> artifacts;
    vector<string> postCmd;


};

class product {
public:
    product() {};
	~product() {};

    string productName;
	string release;
	string version;

    vector<repository> meta;
	vector<string> proditem;
//std::vector<std::unique_ptr<packageRecipe>> 
};


string readJSON(const string& item, const json& j)
{
	string sr("Unset");
 
    auto exists = j.find(item);
	if (exists != j.end()) {
        try {
            sr = j[item].get<std::string>();
	    } catch (const exception& e) {
            cerr << "error: " << e.what() << endl;
        }
	}
    return sr;
}

#define BUILD_DIR "sources"
#define SCRIPT_FETCH "fetch"
void fetch(void)
{
	  std::ofstream script(SCRIPT_FETCH, std::ios_base::binary | std::ios_base::out);

}
#define KALYRA_BANNER "Kalyra Build System"
#define KALYRA_MAJOR "0"
#define KALYRA_MINOR "1"
#define KALYRA_SUB	"2"

#define KALYRA_WORK_DIR "conf"

int main(int argc, char* argv[])
{
    std::ifstream jsonFile;
    auto prod(unique_ptr<product>(new product()));

    unique_ptr<InputParser> options(new InputParser(argc, argv));

    cout <<  termcolor::cyan << KALYRA_BANNER << " v" << KALYRA_MAJOR << "." << KALYRA_MINOR << "." << KALYRA_SUB << termcolor::reset << endl;

    // om 1 arguent anges då anats det vara manifest.. kolla om sista 4 är json?
	// env-setup <manifest> -> .kalyra-manifest fetchar sources
	// product-setup <manifest>
	// product-build -b <recept> -m <meta lager>

   if (options->showHelp()){
        cout << "Available command options:" << endl;
        cout << "-m, --manifest <name>   : Project manifest file (mandatory)." << endl;
        cout << OPT_SHORT_FETCH << ", " << OPT_LONG_FETCH << " <recipe>    : " << OPT_DESC_FETCH << endl;
        cout << OPT_SHORT_BUILD << ", " << OPT_LONG_BUILD << " <recipe>    : " << OPT_DESC_BUILD << endl;
        cout << OPT_SHORT_UPDATE << ", " << OPT_LONG_UPDATE << " <recipe>   : " << OPT_DESC_UPDATE << endl;
        cout << "-c, --clean             : Clean working directory" << endl;
        cout << OPT_SHORT_RECIPES << ", " << OPT_LONG_RECIPES << "           : " << OPT_DESC_RECIPES << endl;
        cout << OPT_SHORT_GENERATE << ", " << OPT_LONG_GENERATE << "          : " << OPT_DESC_GENERATE << endl;
        cout << OPT_FWRT << "                  : Firmware Release Tool. Generate a official release after building all components." << endl;
        cout << "--yes,                  : Don't stop and wait for user input, assume yes on all." << endl;
        return EXIT_SUCCESS;
    }


    if (!std::filesystem::exists(KALYRA_WORK_DIR) && !std::filesystem::create_directory(KALYRA_WORK_DIR))
	{
        cerr << termcolor::red << "Failed to create work directory." << termcolor::reset << endl;
        return EXIT_FAILURE;
    }


    // nya kataloger skapas av git själv behöver inte göra sources, conf

    jsonFile.open("../x7fw-manifest.json", std::ifstream::in);
    if (!jsonFile) {
	   cerr << "cant open file";
       exit(1);
    }

    json manifest = json::parse(jsonFile);

    jsonFile.close();

    prod->productName.assign( readJSON("name", manifest) );
    prod->release.assign( readJSON("release", manifest) );
    prod->version.assign( readJSON("version", manifest) );

    cout << "manifest: " << prod->productName << " " << prod->release << " " << prod->version << endl;

    // ---------------------------------------

     // meta layers
	auto layers = manifest["layers"];

    if (layers.size() == 0) {
		cout << "no layers found" << endl;
		return(1);
	}
/*
vector<unique_ptr<packageRecipe>> recipes;
    try {
        recipes = manifest::loadRecipes(manifest);
    } catch (const exception& e) {
*/
	for (auto& l: json::iterator_wrapper(layers)) {
        repository meta(
			l.value()[JSON_META_NAME].get<std::string>(),
		    l.value()[JSON_META_URL].get<std::string>(),
			l.value()[JSON_META_VERSION].get<std::string>());

			prod->meta.emplace_back(meta);
   	}
    cout << termcolor::green << "registered " << prod->meta.size() << " layers:" << termcolor::reset << endl;

    for (auto& l : prod->meta) {
        cout << l.name << "@"<< l.url << " " << l.version << endl;
	}

    // so - got the layers.
	// move into next state 
	//
	// 1. fetch - generate fetch bash script
	// 2. ...

    // product items
	auto items = manifest["configs"];

    if (items.size() == 0) {
		cout << "no product items found" << endl;
		return(1);
	}

	for (auto& i: json::iterator_wrapper(items)) {
        auto item(i.value()[JSON_CONF_NAME].get<std::string>());
        prod->proditem.emplace_back(item);
   	}
    cout << termcolor::green << "registered " << prod->proditem.size() << " product items:" << termcolor::reset << endl;

    for (auto& i : prod->proditem) {
        cout << i << endl;
	}

// NU parsa prodconfar
// -----------------------------------------------------------------------------------------------

    //jsonFile.open("conf/" + prod->meta[0].name + "/" + prod->proditem[0], std::ifstream::in);
    jsonFile.open("conf/meta-hiab/x7fw-conf.json", std::ifstream::in);
    if (!jsonFile) {
	   cerr << "cant open file";
       exit(1);
    }

    auto prodConf(unique_ptr<productConfiguration>(new productConfiguration()));

    json prodItem = json::parse(jsonFile);

    jsonFile.close();

	auto recipes = prodItem["recipes"];

    if (recipes.size() == 0) {
		cout << "no recipes items found" << endl;
		return(1);
	}

	for (auto& r: json::iterator_wrapper(recipes))
	{
        repository repo(
			r.value()[JSON_SRC_NAME].get<std::string>(),
		    r.value()[JSON_SRC_URL].get<std::string>(),
			r.value()[JSON_SRC_VERSION].get<std::string>());

        prodConf->recipes.emplace_back(repo);
   	}
    cout << termcolor::green << "registered " << prodConf->recipes.size() << " recipes items:" << termcolor::reset << endl;

    for (auto& i : prodConf->recipes) {
        cout << i.name << " @" << i.url << " " <<  i.version << endl;
	}


	auto packages = prodItem["packages"];
	for (auto& r: json::iterator_wrapper(packages))
	{
		string rp;
		string override("");
		string target("");
	
		rp.assign(r.value()[JSON_PKG_RECIPE].get<std::string>());

		if (r.value()[JSON_PKG_REVISION].empty() == false)
		{
			override.assign(r.value()[JSON_PKG_REVISION].get<std::string>());
		}

		if (r.value()[JSON_PKG_TARGET].empty() == false)
		{
			override.assign(r.value()[JSON_PKG_TARGET].get<std::string>());
		}

		package pack(rp, target, override);

        prodConf->packages.emplace_back(pack);
   	}

    cout << termcolor::green << "registered " << prodConf->packages.size() << " package items:" << termcolor::reset << endl;

    for (auto& i : prodConf->packages) {
        cout << i.recipe << " ovr:" << i.override << " targ:" <<  i.target << endl;
	}

	// artifacts
	//auto cmdPre = prodItem["pre-commands"];
	
	//for (auto it = cmdPre.begin(); it != cmdPre.end(); ++it)

	vector<string> releaseComp;
	releaseComp.emplace_back("genie");
	releaseComp.emplace_back("spacex7plt");



	auto arts = prodItem["artifacts"];

	auto cmdPre = arts["pre-commands"].get<std::vector<string>>();
	for (auto i : cmdPre)
     {
         std::cout << i << ' ';
     } 

	for (auto rec : releaseComp)
	{
		auto rc = arts[rec];

    	if (rc.size() == 0) {
			cout << "no artifacts found" << endl;
			return(1);
		}

		for (auto& fc: json::iterator_wrapper(rc))
		{
			cout << "copy "	<< fc.value()["src"].get<std::string>() << " -> ";
		    cout << fc.value()["dest"].get<std::string>() << endl;

   		}

	}


	// recipes --------------------------------------------------------------------------

    jsonFile.open("conf/recipes-hiab/spacex7platform-recipe.json", std::ifstream::in);
    if (!jsonFile) {
	   cerr << "cant open file";
       exit(1);
    }

    auto packRecipe(unique_ptr<packageRecipe>(new packageRecipe()));

    json recipeItem = json::parse(jsonFile);

    jsonFile.close();

	packRecipe->name.assign( readJSON("name", recipeItem) );
		cout << "recipe: " << packRecipe->name << endl;

	auto dep = recipeItem["depends"].get<std::vector<string>>();
    for (auto d : dep) {
	    packRecipe->dependency.emplace_back(d);
		cout << "dep: " << d << endl;
	}

	packRecipe->target.assign( readJSON("target", recipeItem) );

	auto cmds = recipeItem[packRecipe->target].get<std::vector<string>>();
    for (auto c : cmds) {
	    packRecipe->cmdList.emplace_back(c);
	}

    for (auto c : packRecipe->cmdList) {
		cout << "TARG " << c << endl;
	}


// generera byggscripten


/*    if (!std::filesystem::exists(BUILD_DIR) && !std::filesystem::create_directory(BUILD_DIR)) {
        cerr << termcolor::red << "Failed to create build directory." << termcolor::reset << endl;
        return EXIT_FAILURE;
    }*/
    cout << "Firmware Factory: ";



//https://github.com/nlohmann/json/issues/1369


	return 0;
}
