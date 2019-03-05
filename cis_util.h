#pragma once

#include <string>
#include <vector>

#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

struct job
{
    job(std::string name);
    std::string name;
    pt::ptree to_ptree();
};

class job_list
{
    const std::string cis_jobs_path_;
    std::vector<job> jobs_;
public:
    job_list(const std::string& path);
    pt::ptree to_ptree();
    std::string to_json_string();
    void fetch();
};
