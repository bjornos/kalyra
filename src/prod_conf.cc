#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>

#include <nlohmann/json.hpp>
#include "termcolor/termcolor.hpp"

#include "prod_conf.hh"
#include "recipe.hh"
#include "kalyra.hh"

#include "manifest.hh"

using namespace std;
using json = nlohmann::json;

#if 0

void product::prod_config()
{
    std::ifstream json_file;

    json_file.open("conf/meta-hiab/x7fw-conf.json", std::ifstream::in);
    if (!json_file) {
	   cerr << termcolor::red << "Unable to open file <file>" << termcolor::reset << endl;
       return ; //null THROW
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
       // cout << i.recipe << " ovr:" << i.override << " targ:" <<  i.target << endl;
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
			// throw return(1);
            return;
		}

		for (auto& fc: json::iterator_wrapper(rc))
		{
			cout << "copy "	<< fc.value()["src"].get<std::string>() << " -> ";
		    cout << fc.value()["dest"].get<std::string>() << endl;

   		}

	}



}



#endif