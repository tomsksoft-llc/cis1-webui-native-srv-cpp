#include "cis_util.h"

#include <filesystem>
#include <vector>

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
