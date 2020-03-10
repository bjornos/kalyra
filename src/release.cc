//#include <ifstream>
#include <fstream>
#include <sstream>

#include "termcolor/termcolor.hpp"

#include "kalyra.hh"
#include "release.hh"
#include "manifest.hh"

using namespace std;
using json = nlohmann::json;

static bool file_exists(const string& file_name)
{
    std::ifstream infile(file_name);
    return infile.good();
}


const std::string release::get_name() noexcept
{
    return this->name;
}

vector<unique_ptr<product>> release::get_products()
{
    vector<unique_ptr<product>> sw_release;
    std::ifstream json_file;

    for (auto& p : this->products)
	{
        string prodconf;
        bool parsed = false;
        auto product_entity(unique_ptr<product>(new product()));

        for (auto& m : this->meta)
		{
			auto meta = m.get_name();
			prodconf.assign(KALYRA_CONF_DIR "/" + meta + "/" + p);

            if (file_exists(prodconf))
			{
                json_file.open(prodconf, std::ifstream::in);

		        if (!json_file) {
	                cerr << termcolor::red << "Unable to open file " << prodconf << " for reading." << termcolor::reset << endl;
                    //return EXIT_FAILURE;
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
                    product_entity->directories = manifest::product_get_dirs(prod_item);
				} catch (exception& e) {
					// dirs are optional
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
            throw logic_error("Failed to locate configuration " + p);
			//cerr << termcolor::red << "Failed to locate configuration " << p << termcolor::reset << endl;
            //return EXIT_FAILURE;
		}
	}

    return sw_release;

}


void release::init()
{
    this->name = manifest::get_header_item(this->manifest, "name");
    if (this->name.empty()) {
        throw logic_error("name not found.");
    }

    this->version = manifest::get_header_item(this->manifest, "version");
    if (this->version.empty()) {
        throw logic_error("version not found.");
    }

    this->stage = manifest::get_header_item(this->manifest, "stage");
    if (this->stage.empty()) {
        throw logic_error("stage not found.");
    }

    this->build = manifest::get_header_item(this->manifest, "build");
    if (this->build.empty()) {
        throw logic_error("build not found.");
    }

    try {
        this->meta = manifest::release_get_meta(manifest);
    } catch (const exception& e) {
        throw logic_error("Parsing meta-layers failed.");
    }

    try {
		this->products = manifest::release_get_products(manifest);
	} catch (exception& e) {
        throw logic_error("Parsing product configurations failed.");
	}


}



release::release(const nlohmann::json& product_manifest) :
    manifest(product_manifest)
{
}
