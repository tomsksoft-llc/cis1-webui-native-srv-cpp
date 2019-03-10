#include "cis_util.h"

#include <filesystem>
#include <iostream>

#include <boost/process.hpp>

#include "child_process.h"
#include "cis_dirs.h"

namespace fs = std::filesystem;

project::project(std::string n)
    : name(std::move(n))
{}

pt::ptree project::to_ptree()
{
    pt::ptree result;
    result.put("", name);
    return result; 
}

project_list::project_list(const std::string& path)
    : cis_projects_path_(path)
{
}

void project_list::fetch()
{
    projects_.clear();
    try
    {
        for(auto& p: fs::directory_iterator(cis_projects_path_))
        {
            if(p.is_directory())
            {
                projects_.emplace_back(p.path().filename().string());
            }
        }
    }
    catch(std::filesystem::filesystem_error& err)
    {}
}

pt::ptree project_list::to_ptree()
{
    pt::ptree result;
    for(auto& project : projects_)
    {
        result.push_back(std::make_pair("", project.to_ptree()));
    }
    return result;
}

std::string project_list::to_json_string()
{
    pt::ptree json_tree;
    json_tree.add_child("projects", to_ptree());
    std::stringstream json;
    pt::write_json(json, json_tree);
    return json.str();
}

void run_job(
        boost::asio::io_context& ctx,
        const std::string& project,
        const std::string& name)
{
    auto env = boost::this_process::environment();
    env["cis_base_dir"] = cis::get_root_dir();
    std::make_shared<child_process>(ctx, env)->run(
            "sh",
            {"startjob", project + "/" + name},
            [](int exit, std::vector<char>& buffer, const std::error_code& ec)
            {
                if(!ec)
                {
                    std::cout << "process exited with " << exit << std::endl;
                    std::cout << "std_out contain:" << std::endl;
                    std::cout.write(buffer.data(), buffer.size());
                }
                else
                {
                    std::cout << "error" << std::endl;
                }
            });
}
