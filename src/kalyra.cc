#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <list>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif


#include "termcolor/termcolor.hpp"
#include "input_parser.hh"

#include "kalyra.hh"

#include "repository.hh"
#include "release.hh"
#include "script_generator.hh"
#include "prod_conf.hh"
#include "recipe.hh"
#include "manifest.hh"

namespace fs = std::filesystem;

using namespace std;
using json = nlohmann::json;

bool file_exists(const string& file_name)
{
    std::ifstream infile(file_name);
    return infile.good();
}

int error_out(const string what)
{
    cerr << termcolor::red << "Error: " << what << termcolor::reset  << endl;
    exit(EXIT_FAILURE);
}


int main(int argc, char* argv[])
{
    unique_ptr<InputParser> options(new InputParser(argc, argv));
    string manifest_product;
    std::ifstream json_file;
    vector<unique_ptr<product>> sw_release;


    if (argc < 2) {
		cout << "Usage: kalyra <manifest>" << endl;
		return 1;
	}

    cout <<  termcolor::cyan << KALYRA_BANNER << " v" << KALYRA_MAJOR << "." << KALYRA_MINOR << "." << KALYRA_SUB << termcolor::reset << endl;

    if (options->showHelp()){
    	cout << "Available command options:" << endl;
        cout << "-m, --manifest <name>   : Project manifest file (mandatory)." << endl;
        cout << "-c, --clean             : Clean working directory" << endl;
        cout << "-b  --build <recipe>    : Build a recipe  (only works with single builds)   " << endl;
        cout << "-v  --verbose           : Show compile/build output   " << endl;
    	return EXIT_SUCCESS;
    }

    if (options->clean())
	{
        return (fs::remove_all(KALYRA_CONF_DIR) || 
		        fs::remove_all(KALYRA_SCRIPT_DIR) ||
                fs::remove_all("artifacts") || 
                fs::remove_all("sources") ||
                fs::remove_all("log") ||
                fs::remove(".kalyra-manifest") );

    }


	manifest_product.assign(argv[1]);
	if (manifest_product.empty())
	{
		cerr << termcolor::red << "Missing product manifest" << termcolor::reset << endl;
	}

    json_file.open(manifest_product, std::ifstream::in);
    if (!json_file) {
        cerr << termcolor::red << "Unable to open" << manifest_product << " for reading." << termcolor::reset << endl;
        exit(1);
    }
	else
	{
		cout << "Processing " << manifest_product << endl;
	}

    auto manifest = json::parse(json_file);

    json_file.close();

    auto product_release(unique_ptr<release>(new release(manifest)));
 
    try {
        product_release->init();
    } catch (exception& e) {
        error_out(e.what());
    }
	
    //cout << "Release Configuration: " << product_release->get_name() << " " << product_release->version << product_release->stage << product_release->build << endl;

    cout << termcolor::green << "Registered " << product_release->meta.size() << " layer(s)." << termcolor::reset << endl;

    for (auto& l : product_release->meta) {
        DBG(cout << "[" << l.get_name() << "]" << endl);
	}

    try {
        fs::create_directory(KALYRA_SCRIPT_DIR);
        fs::create_directory("log");
	} catch (exception& e) {
		error_out(e.what());
    }    


    script_generator::fetch(product_release->meta, SCRIPT_FETCH_META, KALYRA_CONF_DIR);

    try {
        script_generator::run_script(CMD_SCRIPT_FETCH_META);
    } catch (const exception& e) {
        error_out("Fetch mate layer failed.");
    }


    try {
        sw_release = product_release->get_builds();
	} catch (exception& e) {
		error_out(e.what());
	}

    for (auto& swrel : sw_release)
	{
		cout << termcolor::cyan << "Including " << swrel->name << " in build." << termcolor::reset << endl;
	}
   
    cout << "Setting up recipe(s)..." << endl;

    for (auto& swrel : sw_release)
	{
        // fetch for each product - needed to get correct git revision
        script_generator::fetch(swrel->recipes, SCRIPT_FETCH_RECIPE, KALYRA_CONF_DIR);

        if (script_generator::run_script(CMD_SCRIPT_FETCH_RECIPE) == false)
        {
            cout << termcolor::red << "Error." << termcolor::reset <<  endl;
	        return EXIT_FAILURE;
        }
	}

    cout << "Fetching package(s)..." << endl;

    auto package_recipe(unique_ptr<recipe>(new recipe()));
    bool recipe_found = false;
    string recipe_current;
    vector<string> fetch_scripts;
	vector<string> build_scripts;
	vector<string> release_scripts;
    vector<repository> product_fetch;

	vector<tuple<string, string, string, string>> product_script;
    vector<unique_ptr<recipe>> product_recipes;

    for (auto& swrel : sw_release)
	{
		cout << termcolor::yellow << "Setting up " << swrel->name << termcolor::reset << endl;


         for (auto& p : swrel->packages)
		 {
            recipe_found = false;

            for (auto& r : swrel->recipes)
			{
			    recipe_current.assign(KALYRA_CONF_DIR  PLT_SLASH + r.get_name() + PLT_SLASH + p.recipe + "_recipe.json");

                if (file_exists(recipe_current)) {
					recipe_found = true;
				    continue;
				}
			}
			
            if (recipe_found) // current != null instead
			{
                json_file.open(recipe_current, std::ifstream::in);

		        if (!json_file) {
	                cerr << termcolor::red << "Unable to open file " << recipe_current << " for reading." << termcolor::reset << endl;
                    return EXIT_FAILURE;
		        }

                auto prod_item = json::parse(json_file);

                json_file.close();

                package_recipe = manifest::parse_recipe(prod_item);

                DBG(cout << "Added " << package_recipe->name << endl);

                // Adjust for project overrides
                if (p.override.empty() == false)
				{
                    package_recipe->revision = p.override;
                }
                if (p.target.empty() == false)
				{
                    package_recipe->target = p.target;
					package_recipe->cmd_list = manifest::parse_recipe_target(prod_item, package_recipe->target);
                }
		

                repository new_repo(package_recipe->name, package_recipe->url, package_recipe->revision);
				product_fetch.emplace_back(new_repo);

				product_recipes.emplace_back(move(package_recipe));
            }
			else
			{
                cerr << termcolor::red << "Unable to locate recipe for " << p.recipe << termcolor::reset << endl;
                return EXIT_FAILURE;
			}
			
		 }
    
// FIXME: move to scriptgenerator
#if defined(_WIN32) || defined(_WIN64)
        const string script_fetch_product(KALYRA_SCRIPT_DIR "\\fetch_" + swrel->name + ".bat");
        const string script_build_product(KALYRA_SCRIPT_DIR "\\build_" + swrel->name + ".bat");
        const string script_release_product(KALYRA_SCRIPT_DIR "\\install_" + swrel->name + ".bat");
#else 
        const string script_fetch_product(KALYRA_SCRIPT_DIR "/fetch_" + swrel->name + ".sh");
        const string script_build_product(KALYRA_SCRIPT_DIR "/build_" + swrel->name + ".sh");
        const string script_release_product(KALYRA_SCRIPT_DIR "/install_" + swrel->name + ".sh");
#endif
        script_generator::fetch(product_fetch, script_fetch_product, "sources");
		script_generator::build(product_recipes, script_build_product, "sources", options->verbose());
		script_generator::release(swrel, script_release_product, "sources");

        swrel->package_recipes = move(product_recipes);
    
		product_script.emplace_back(make_tuple(script_fetch_product, script_build_product, script_release_product, swrel->name));

		product_fetch.clear();
		product_recipes.clear();

	}

    for (auto& ps : product_script)
    {
        //
        // fetch
        //
        try {
            script_generator::run_script(get<0>(ps));
        } catch (const exception& e) {
            error_out("Failed to fetch repository - see log for details.");
        }
#if 1
        //
        // build
        //
        try {
            script_generator::run_script(get<1>(ps));
        } catch (const exception& e) {
            error_out("Failed to build package - see log for details.");
        }

#endif
        //
        // install
        //
        try {
            script_generator::run_script(get<2>(ps));
        } catch (const exception& e) {
            error_out("Failed to install - see log for details.");
        }

	}

    //
    //  Generate revision logs
    //
    for (auto& build : sw_release)
	{
#if defined(_WIN32) || defined(_WIN64)
        const string log_path("artifacts\\" + build->name);
        std::ofstream log(log_path + "\\" + LOG_BUILD_REVISIONS, std::ios_base::binary | std::ios_base::out);
#endif

        log << "Kalyra: "  << KALYRA_MAJOR << "." << KALYRA_MINOR << "." << KALYRA_SUB << endl;
        log << "Manifest: " << "change to argv[0]" << ": *FIXME git hash*" << endl;

        for (auto& repo : build->recipes)
        {
            log << repo.get_name() << ": ";

            if (repo.get_rev().empty())
            {
                log << "master" << endl;
            }
            else
            {
                log << repo.get_rev() << endl;
            }
        }

        for (auto& package : build->package_recipes)
        {
            log << package->name << ": ";

            if (package->revision.empty())
            {
                log << "master" << endl;
            }
            else
            {
                log << package->revision << endl;
            }
        }


        log.close();

    }

    return EXIT_SUCCESS;
}
