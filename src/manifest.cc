#include "manifest.hh"
#include "kalyra.hh"

using namespace std;

bool fileExists(const string& fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

const cJSON* manifest::getValue(const cJSON* recipe, string tag)
{
    const cJSON* entry;

    if (cJSON_HasObjectItem(recipe, tag.c_str())) {
        entry = cJSON_GetObjectItemCaseSensitive(recipe, tag.c_str());
    } else {
        cerr << "Warning: Could not find component '" << tag << "'." << endl;
        entry = cJSON_CreateString("Unknown");
    }

    return entry;
}

void manifest::loadHeader(const cJSON*& m, const string& manifest)
{
    std::ifstream t(manifest);
    std::stringstream buffer;

    if (!fileExists(manifest))
        throw std::invalid_argument("Manifest file not found.");

    buffer << t.rdbuf();

    m = cJSON_Parse(buffer.str().c_str());

    if (m == NULL) {
        auto errPtr = cJSON_GetErrorPtr();
        if (errPtr != NULL) {
            cout << "[DBG] Error before: " << errPtr << endl;
        }
        throw std::invalid_argument("Error parsing manifest.");
    }
}

vector<unique_ptr<packageRecipe>> manifest::loadRecipes(const cJSON* manifest)
{
    const cJSON* package;
    vector<unique_ptr<packageRecipe>> recipes;

    auto packages = cJSON_GetObjectItemCaseSensitive(manifest, "packages");
    cJSON_ArrayForEach(package, packages)
    {
        string revOverride("");
        string targetOverride("");

        auto recipe = cJSON_GetObjectItemCaseSensitive(package, "name");
        auto rev = cJSON_GetObjectItemCaseSensitive(package, "revision");
        auto target = cJSON_GetObjectItemCaseSensitive(package, "target");

        if (!cJSON_IsString(recipe) || (recipe->valuestring == NULL)) {
            throw std::invalid_argument("Error parsing targets.");
        }

        if (cJSON_IsString(rev) && (rev->valuestring != NULL))
            revOverride = rev->valuestring;

        if (cJSON_IsString(target) && (target->valuestring != NULL))
            targetOverride = target->valuestring;

        auto p(unique_ptr<packageRecipe>(new packageRecipe(recipe->valuestring, revOverride, targetOverride)));

        recipes.emplace_back(move(p));
    }

    //
    // recursive parse any imported manifest for additional recipes
    //
    bool importManifest = true;
    vector<string> imports;

    while (importManifest == true) {
        auto imp = cJSON_GetObjectItemCaseSensitive(manifest, "import");
        if (imp == NULL) {
            // No import field
            importManifest = false;
            continue;
        }

        for (auto& i : imports) {
            if (i.compare(imp->valuestring) == 0) {
                // Imported already
                importManifest = false;
                continue;
            }
        }

        if (importManifest) {
            const cJSON* import;
            imports.emplace_back(imp->valuestring);

            manifest::loadHeader(import, imp->valuestring);
            vector<unique_ptr<packageRecipe>> importRecipe;

            importRecipe = manifest::loadRecipes(import);
            for (auto& r : importRecipe) {
                recipes.emplace_back(move(r));
            }
        }
    }

    return recipes;
}

unique_ptr<releaseComponent> manifest::loadComponents(std::vector<std::unique_ptr<packageRecipe>>& recipes, const cJSON* manifest)
{
    const cJSON* component;
    vector<string> preCommands;
    vector<string> postCommands;
    vector<string> releaseFiles;

    auto components = cJSON_GetObjectItemCaseSensitive(manifest, "release-components");

    auto section = cJSON_GetObjectItemCaseSensitive(components, "pre-commands");
    if (section != NULL) {
        cJSON_ArrayForEach(component, section)
        {
            preCommands.emplace_back(component->valuestring);
        }
    }

    section = cJSON_GetObjectItemCaseSensitive(components, "post-commands");
    if (section != NULL) {
        cJSON_ArrayForEach(component, section)
        {
            postCommands.emplace_back(component->valuestring);
        }
    }

    for (auto& target : recipes) {
        const cJSON* releaseFile;
        auto t = cJSON_GetObjectItem(components, target->getName().c_str());
        if (t != NULL) {
            cJSON_ArrayForEach(releaseFile, t) {
                releaseFiles.emplace_back(target->getRoot() + PLT_SLASH + releaseFile->valuestring);
            }
        }
    }

    auto rc(unique_ptr<releaseComponent>(new releaseComponent(preCommands, postCommands, releaseFiles)));

    return move(rc);
}
