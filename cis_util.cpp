#include "cis_util.h"

#include <filesystem>
#include <vector>

#include <boost/property_tree/json_parser.hpp>

namespace fs = std::filesystem;
namespace pt = boost::property_tree;

job_list::job_list(const std::string& path)
    : cis_jobs_(path)
{
}

void job_list::fetch()
{
    pt::ptree children;
    for(auto& p: fs::directory_iterator(cis_jobs_))
    {
        if(p.is_directory())
        {
            pt::ptree job;
            job.put("", p.path().filename().string());
            children.push_back(std::make_pair("", job));
        }
    }
    tree_.add_child("jobs", children);
}

std::string job_list::to_string()
{
    std::stringstream json;
    pt::write_json(json, tree_);
    return json.str();
}
