#include "manifest.hh"


using namespace std;
using json = nlohmann::json;

static bool JSON_exists(const string& key, const json& file)
{
   auto exists = file.find(key);

   return (exists != file.end());
}

static string JSON_read(const string& item, const json& j)
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



string manifest::get_header_item(const json& manifest, string item)
{
    return JSON_read(item, manifest);
}

vector<repository> manifest::release_get_meta(const json& manifest)
{
    vector<repository> local_meta;

     // meta layers
	auto layers = manifest["layers"];

    if (layers.size() == 0) {
        throw logic_error("No meta layer found");
	}

    for (auto& l: json::iterator_wrapper(layers)) {
        repository meta(l.value()[JSON_REPO_NAME].get<std::string>(),
		                l.value()[JSON_REPO_URL].get<std::string>(),
                        l.value()[JSON_REPO_VERSION].get<std::string>());

        local_meta.emplace_back(meta);
   	}

    return local_meta;
}

vector<string> manifest::release_get_products(const json& manifest)
{
    vector<string> products;

	auto items = manifest["configs"];

    if (items.size() == 0) {
        throw logic_error("No product item found.");
	}

	for (auto& i: json::iterator_wrapper(items)) {
        auto item(i.value()[JSON_CONF_NAME].get<std::string>());
        products.emplace_back(item);
   	}

    return products;
}


std::vector<repository> manifest::product_get_recipes(const json& manifest)
{
    std::vector<repository> recipe;

	auto recipes = manifest["recipes"];

    if (recipes.size() == 0) {
        throw logic_error("No recipes item found.");
	}

	for (auto& r: json::iterator_wrapper(recipes))
	{
        repository repo(
			r.value()[JSON_SRC_NAME].get<std::string>(),
		    r.value()[JSON_SRC_URL].get<std::string>(),
			r.value()[JSON_SRC_VERSION].get<std::string>());

        recipe.emplace_back(repo);
   	}

    return recipe;
}

std::vector<package> manifest::product_get_packages(const json& manifest)
{
    vector<package> product_packages;

	auto packages = manifest["packages"];

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
			target.assign(r.value()[JSON_PKG_TARGET].get<std::string>());
		}

		package pack(rp, target, override);

        product_packages.emplace_back(pack);
   	}

    return product_packages;
}



std::vector<string> manifest::product_get_cmd(const json& manifest, const string& cmd_type)
{
    //json cmd;
    vector<string> cmd;

   if (JSON_exists("commands", manifest))
   {
        auto cmds = manifest["commands"];

//        auto subcmd = cmds.find(cmd_type);
//	    if (subcmd != cmds.end())
        if (JSON_exists(cmd_type, cmds))
        {
            cmd = cmds[cmd_type].get<std::vector<string>>();
        }
    }


    
    return cmd;
}

/*	for (auto i : cmdPre)
     {
         std::cout << i << ' ';
     } 
*/

std::vector<artifact> manifest::product_get_artifacts(const json& manifest, const std::vector<package>& packages)
{
    vector<artifact> a_items;
    json artifacts;

    if (JSON_exists("artifacts", manifest)) {
        artifacts = manifest["artifacts"];
    } else {
        return a_items;
    }

	for (auto& p : packages)
	{
        if (JSON_exists(p.recipe, artifacts))
        {
            auto artifact_entry = artifacts[p.recipe];

            artifact a_item;
            a_item.target = p.recipe;

            for (auto& files: json::iterator_wrapper(artifact_entry))
		    {
			    //cout << "copy "	<< files.value()["src"].get<std::string>() << " -> ";
		        //cout << files.value()["dest"].get<std::string>() << endl;
                a_item.item.emplace_back(make_tuple(files.value()["src"].get<std::string>(),files.value()["dest"].get<std::string>()));
            }
            a_items.emplace_back(a_item);
        }
    }

    return a_items;
}


std::vector<std::string> manifest::parse_recipe_target(const json& recipe_file, const string& target)
{
   std::vector<std::string> cmd_list;

    if (JSON_exists(target, recipe_file))
    {
        auto cmds = recipe_file[target].get<std::vector<string>>();
        for (auto c : cmds)
        {
	        cmd_list.emplace_back(c);
	    }

    }

    return cmd_list;
}



unique_ptr<recipe> manifest::parse_recipe(const json& recipe_file)
{
    auto this_recipe(unique_ptr<recipe>(new recipe()));    

    auto package = recipe_file["package"]; //.get<std::vector<string>>();

	this_recipe->name.assign(JSON_read("name", package));
	this_recipe->url.assign(JSON_read("url", package));
	this_recipe->revision.assign(JSON_read("revision", package));
	this_recipe->root.assign(JSON_read("root", package));
	this_recipe->license.assign(JSON_read("license", package));

    if (JSON_exists("depends", package))
    {
	    auto dependencies = package["depends"].get<std::vector<string>>();
        for (auto& d : dependencies)
        {
	        this_recipe->dependency.emplace_back(d);
        }
	}
	this_recipe->target.assign(JSON_read("target", package));

    this_recipe->cmd_list = parse_recipe_target(recipe_file, this_recipe->target);

    return move(this_recipe);
}

