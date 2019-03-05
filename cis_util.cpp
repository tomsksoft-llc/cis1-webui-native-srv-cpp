#include "cis_util.h"

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

job::job(std::string n)
    : name(std::move(n))
{}

pt::ptree job::to_ptree()
{
    pt::ptree result;
    result.put("", name);
    return result; 
}

job_list::job_list(const std::string& path)
    : cis_jobs_path_(path)
{
}

void job_list::fetch()
{
    jobs_.clear();
    try
    {
        for(auto& p: fs::directory_iterator(cis_jobs_path_))
        {
            if(p.is_directory())
            {
                jobs_.emplace_back(p.path().filename().string());
            }
        }
    }
    catch(std::filesystem::filesystem_error& err)
    {}
}

pt::ptree job_list::to_ptree()
{
    pt::ptree result;
    for(auto& job : jobs_)
    {
        result.push_back(std::make_pair("", job.to_ptree()));
    }
    return result;
}

std::string job_list::to_json_string()
{
    pt::ptree json_tree;
    json_tree.add_child("jobs", to_ptree());
    std::stringstream json;
    pt::write_json(json, json_tree);
    return json.str();
}
