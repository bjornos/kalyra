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

/*string readJSON(const string& item, const json& j)
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
}*/


bool file_exists(const string& file_name)
{
    std::ifstream infile(file_name);
    return infile.good();
}





int main(int argc, char* argv[])
{
    std::ifstream jsonFile;
    unique_ptr<InputParser> options(new InputParser(argc, argv));
    string manifest_product;
    std::ifstream json_file;

    auto project_release(unique_ptr<release>(new release()));
    vector<unique_ptr<product>> sw_release;


    if (argc < 2) {
		cout << "Usage: kalyra <manifest>" << endl;
		return 1;
	}

    cout <<  termcolor::cyan << KALYRA_BANNER << " v" << KALYRA_MAJOR << "." << KALYRA_MINOR << "." << KALYRA_SUB << termcolor::reset << endl;

    if (options->clean())
	{
        return (fs::remove_all(KALYRA_CONF_DIR) || 
		        fs::remove_all(KALYRA_SCRIPT_DIR) ||
                fs::remove_all("artifacts") || 
                fs::remove_all("sources") ||
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

/*    try {
        project_release->name = manifest::get_header_item(manifest, "name");
        project_release->version = manifest::get_header_item(manifest, "version");
        project_release->stage = manifest::get_header_item(manifest, "stage");
        project_release->build = manifest::get_header_item(manifest, "build");
	} catch (exception& e) {
		cerr << termcolor::red << "Error processing manifest: " << termcolor::reset << e.what() << endl;
	} */

    try {
        project_release->load_header(manifest);
	} catch (exception& e) {
		cerr << termcolor::red << "Error processing manifest: " << termcolor::reset << "Could not find '" << e.what() << "'" << endl;
        return EXIT_FAILURE;
	}
    
	
	cout << "Release: " << project_release->name << " " << project_release->version << project_release->stage << project_release->build << endl;

	cout << "Setting up layer(s)..." << endl;

    try {
        project_release->meta = manifest::release_get_meta(manifest);
	} catch (exception& e) {
		cerr << termcolor::red << "Error processing manifest: " << termcolor::reset << e.what() << endl;
	}

    cout << termcolor::green << "Registered " << project_release->meta.size() << " layers:" << termcolor::reset << endl;

    for (auto& l : project_release->meta) {
        cout << "[" << l.get_name() << "]" << endl;
	}


//    if (!std::filesystem::exists(KALYRA_SCRIPT_DIR) && !std::filesystem::create_directory(KALYRA_SCRIPT_DIR))
    if (!fs::exists(KALYRA_SCRIPT_DIR) && !fs::create_directory(KALYRA_SCRIPT_DIR))
	{
        cerr << termcolor::red << "Failed to create work directory." << termcolor::reset << endl;
        return EXIT_FAILURE;
    }

    script_generator::fetch(project_release->meta, SCRIPT_FETCH_META, KALYRA_CONF_DIR);


    if (script_generator::run_script(CMD_SCRIPT_FETCH_META) == false)
	{
        cout << termcolor::red << "Error." << termcolor::reset <<  endl;
	    return EXIT_FAILURE;
	}


    try {
		project_release->products = manifest::release_get_products(manifest);
	} catch (exception& e) {
		cerr << termcolor::red << "Error processing manifest: " << termcolor::reset << e.what() << endl;
	}

    cout << termcolor::green << "Registered " << project_release->products.size() << " product items:" << termcolor::reset << endl;

    for (auto& p : project_release->products)
	{
        cout << "[" << p << "]" << endl;
	}

//
// Create sub products
//  

    // product::getRecipeSrc
    for (auto& p : project_release->products)
	{
        string prodconf;
        bool parsed;
        auto product_entity(unique_ptr<product>(new product()));

        parsed = false;

        for (auto& m : project_release->meta)
		{
			auto meta = m.get_name();
			prodconf.assign(KALYRA_CONF_DIR "/" + meta + "/" + p);

            if (file_exists(prodconf))
			{
                json_file.open(prodconf, std::ifstream::in);

		        if (!json_file) {
	                cerr << termcolor::red << "Unable to open file " << prodconf << " for reading." << termcolor::reset << endl;
                    return EXIT_FAILURE;
		        }

                json prod_item = json::parse(json_file);

                json_file.close();

                product_entity->name = manifest::get_header_item(prod_item, "product");
                product_entity->version = manifest::get_header_item(prod_item, "version");

	            try {
					product_entity->recipes = manifest::product_get_recipes(prod_item);
				} catch (exception& e) {
					cerr << termcolor::red << e.what() << termcolor::reset << endl;
				}

                cout << termcolor::green << "registered " << product_entity->recipes.size() << " recipes:" << termcolor::reset << endl;

                for (auto& r: product_entity->recipes) {
                    DBG(cout << r.get_name() << " @" << r.get_url() << " " <<  r.get_rev() << endl);
                }

	            try {
					product_entity->packages = manifest::product_get_packages(prod_item);
				} catch (exception& e) {
					cerr << termcolor::red << e.what() << termcolor::reset << endl;
				}

                cout << termcolor::green << "registered " << product_entity->packages.size() << " package items:" << termcolor::reset << endl;

                for (auto& i : product_entity->packages) {
                    DBG(cout << i.recipe << " override:" << i.override << " target:" <<  i.target << endl);
	            }

	            try {
					product_entity->cmd_pre = manifest::product_get_cmd(prod_item, "pre");
					product_entity->cmd_post = manifest::product_get_cmd(prod_item, "post");
				} catch (exception& e) {
					// pre/post commands are optional
				}

	            try {
					product_entity->artifacts = manifest::product_get_artifacts(prod_item, product_entity->packages);
				} catch (exception& e) {
					// artifacts are optional
				}

                parsed = true;

			}
		}



		if (parsed == true)
		{
            sw_release.emplace_back(move(product_entity));
		} else
		{
			cerr << termcolor::red << "Failed to locate configuration " << p << termcolor::reset << endl;
            return EXIT_FAILURE;
		}
	}


    cout << "This release will include the following products:" << endl;

    for (auto& swrel : sw_release)
	{
		cout << swrel->name << endl;
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
		cout << termcolor::blue << "Setting up " << swrel->name << termcolor::reset << endl;


         for (auto& p : swrel->packages)
		 {
            recipe_found = false;

            for (auto& r : swrel->recipes)
			{
			    recipe_current.assign(KALYRA_CONF_DIR "/" + r.get_name() + "/" + p.recipe + "_recipe.json");

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

                cout << "Added " << package_recipe->name << endl;

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
    
 
        const string script_fetch_product(KALYRA_SCRIPT_DIR "/fetch_" + swrel->name + ".sh");
        const string script_build_product(KALYRA_SCRIPT_DIR "/build_" + swrel->name + ".sh");
        const string script_release_product(KALYRA_SCRIPT_DIR "/release_" + swrel->name + ".sh");

        script_generator::fetch(product_fetch, script_fetch_product, "sources");
		script_generator::build(product_recipes, script_build_product, "sources");
		script_generator::release(swrel, script_release_product, "sources");

		product_script.emplace_back(make_tuple(script_fetch_product, script_build_product, script_release_product, swrel->name));

		product_fetch.clear();
		product_recipes.clear();

	}

    for (auto& ps : product_script)
    {
        cout << termcolor::blue << "[Cooking " <<  std::get<3>(ps) << "]" << termcolor::reset << endl;

        // fetch
        DBG(cout << termcolor::yellow << "RUN " << get<0>(ps) << termcolor::reset <<  endl);
        if (script_generator::run_script(get<0>(ps)) == false)
        {
            cout << termcolor::red << "Error." << termcolor::reset <<  endl;
	        return EXIT_FAILURE;
        }
#if 0
        DBG(cout << termcolor::yellow << "RUN " << get<1>(ps) << termcolor::reset <<  endl);

        // build
        if (script_generator::run_script(get<1>(ps)) == false)
        {
            cout << termcolor::red << "Error." << termcolor::reset <<  endl;
	        return EXIT_FAILURE;
        }
#endif
        // install
        DBG(cout << termcolor::yellow << "RUN " << get<2>(ps) << termcolor::reset <<  endl);

        if (script_generator::run_script(get<2>(ps)) == false)
        {
            cout << termcolor::red << "Error." << termcolor::reset <<  endl;
	        return EXIT_FAILURE;
        }


	}

    return 0;
}
