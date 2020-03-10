#include <sstream>
#include <fstream>
#include <cstdlib>

#include "script_generator.hh"

#if defined(_WIN32) || defined(_WIN64)
constexpr auto PLT_SHELL = "cmd.exe /C ";
#else
constexpr auto PLT_SHELL = "bash ";
#endif


using namespace std;

static const string git_clone(repository& entry, const string& path)
{
#if defined(_WIN32) || defined(_WIN64)
    string clone("@git clone " + entry.get_url() + " -q -b " + entry.get_rev() + " " + path + "\\" + entry.get_name());
#else
    string clone("git clone " + entry.get_url() + " -q -b " + entry.get_rev() + " " + path + "/" + entry.get_name() + " > /dev/null 2>&1");
#endif
    return clone;
}

bool script_generator::run_script(const string& cmd)
{
    const string os_cmd(PLT_SHELL + cmd);

    auto cmd_result = std::system(os_cmd.c_str());
#if defined(_WIN32) || defined(_WIN64)
    if (cmd_result != 0) {
#else
    if (WEXITSTATUS(cmd_result) != 0) {
#endif
        return false;
    }
    return true;
}


void script_generator::fetch(std::vector<repository>& repos, const std::string& script_file, const std::string& path)
{
    std::ofstream script(script_file, std::ios_base::binary | std::ios_base::out);

#if defined(_WIN32) || defined(_WIN64)

    //
    // generate windows bat script
    //

    script << "@echo off" << endl;
    script << "SET ROOTDIR=%cd%" << endl;
    script << "SETLOCAL" << endl;
 
    for (auto& repo : repos)
    {

        script << "IF NOT EXIST " << path << "\\" << repo.get_name() << "\\.git (" << endl;
        script << "rm -rf " << path << "\\" << repo.get_name() << endl;
        script << "echo  ---- Fetching " << repo.get_name() << " rev=" << repo.get_rev() << endl;
        script << git_clone(repo, path) << " || exit 1 " <<  endl;
        script << ") ELSE (" << endl;
        script << "echo # Using " << repo.get_name() << " rev=" << repo.get_rev() << endl;
        script << "cd " << path << "\\" << repo.get_name() << " || exit 1" << endl;

		if (!repo.get_name().empty())
		    script << "@git checkout -q " << repo.get_rev() << " || exit 1 " << endl;

        script << "cd %ROOTDIR%" << endl;
        script << ")" << endl;

    }
    script.close();

#else

    //
    // generate bash script
    //

    script << "#!/bin/sh" << endl;
    script << "rootdir=${PWD}" << endl;

    for (auto& repo : repos)
    {
        script << "if [ ! -d " << path << "/" << repo.get_name() << "/.git ]; then" << endl;
        // remove generated leftovers
        script << "rm -rf " << path << "/" << repo.get_name() << endl;
        script << "echo  ---- Fetching " << repo.get_name() << " rev=" << repo.get_rev() << endl;
        script << git_clone(repo, path) << " || exit 1 " <<  endl;
        script << "else" << endl;
        script << "echo  \"# Using \"" << repo.get_name() << " rev=" << repo.get_rev() << endl;
        script << "cd " << path << "/" << repo.get_name() << " || exit 1" << endl;

		if (!repo.get_name().empty())
		    script << "git checkout " << repo.get_rev() << " > /dev/null 2>&1 || exit 1 " << endl;

        script << "cd ${rootdir}" << endl;
        script << "fi" << endl;
    }
    script.close();
#endif
}

void script_generator::build(const std::vector<std::unique_ptr<recipe>>& recipes, const std::string& script_file, const std::string& path)
{
    std::ofstream script(script_file, std::ios_base::binary | std::ios_base::out);

#if defined(_WIN32) || defined(_WIN64)
    script << "@echo off" << endl;
    script << "SET ROOTDIR=%cd%" << endl;
    script << "SETLOCAL" << endl;
#else
    script << "#!/bin/sh" << endl;
    script << "rootdir=${PWD}" << endl;
#endif

   for (auto& r : recipes)
   {
        script << "echo == Building " << r->name << endl;

        script << "cd " << path << endl;

        script << "cd " << r->name << endl;

        for (auto& cmd : r->cmd_list)
        {
//        script << cmd << " > /dev/null 2>&1 || exit 1" << endl;
            if (cmd.empty() == false)
                script << "@" << cmd << " || exit 1" << endl;
        }

#if defined(_WIN32) || defined(_WIN64)
        script << "cd %ROOTDIR%" << endl;
#else
        script << "cd ${rootdir}" << endl;
#endif
    }

    script.close();
}


//void script_generator::release(const std::vector<std::unique_ptr<recipe>>& recipes, const std::string& script_file, const std::string& path)

// change to install instead of release
void script_generator::release(const std::unique_ptr<product>& prod, const std::string& script_file, const std::string& path)
{
    std::ofstream script(script_file, std::ios_base::binary | std::ios_base::out);

#if defined(_WIN32) || defined(_WIN64)

    const string release_path("artifacts\\" + prod->name + "\\" + prod->version);

    //
    // generate windows bat script
    //

    script << "@echo off" << endl;
    script << "SET ROOTDIR=%cd%" << endl;
    script << "SETLOCAL" << endl;

    script << "echo == Installing " << prod->name << endl;

    script << "IF NOT EXIST %ROOTDIR%\\" << release_path << " (" << endl;
    script << "md %ROOTDIR%\\" << release_path << " || exit 1" << endl;
    script << ")" << endl;


    for (auto& dirs : prod->directories)
    {
        script << "IF NOT EXIST %ROOTDIR%\\" << release_path << "\\" << dirs << " (" << endl;
        script << "md %ROOTDIR%\\" << release_path << "\\" << dirs << " || exit 1" << endl;
        script << ")" << endl;
    }

    for (auto& pre : prod->cmd_pre)
    {
            if (pre.empty() == false)
                script << pre << " || exit 1" << endl;
    }

    for (auto& a : prod->artifacts)
    {
         script << "cd " << path << " || exit 1" << endl;
         script << "cd " << a.target << " || exit 1" << endl;

        for (auto& i : a.item)
        {
            script << "cp -rfv " << get<0>(i) << " %ROOTDIR%\\" << release_path << "\\" << get<1>(i) << " || exit 1" << endl;
        }
         
         script << "cd %ROOTDIR% || exit 1" << endl;
    }


#else
    const string release_path("artifacts/" + prod->name + "/" + prod->version);

    //
    // generate unix bash script
    //

    script << "#!/bin/sh" << endl;
    script << "rootdir=${PWD}" << endl;

    script << "echo == Installing " << prod->name << endl;

    script << "mkdir -p ${rootdir}/" << release_path << " || exit 1" << endl;

    for (auto& pre : prod->cmd_pre)
    {
            if (pre.empty() == false)
                script << pre << " || exit 1" << endl;
    }

    for (auto& a : prod->artifacts)
    {
         script << "cd " << path << " || exit 1" << endl;
         script << "cd " << a.target << " || exit 1" << endl;

        for (auto& i : a.item)
        {
            script << "cp -rf " << get<0>(i) << " ${rootdir}/" << release_path << "/" << get<1>(i) << " || exit 1" << endl;
        }
         
         script << "cd ${rootdir} || exit 1" << endl;
    }

    for (auto& post : prod->cmd_post)
    {
            if (post.empty() == false)
                script << post << " || exit 1" << endl;
    }

    script.close();
#endif

}


