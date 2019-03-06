#pragma once

#include <string>
#include <vector>

#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

struct project
{
    project(std::string name);
    std::string name;
    pt::ptree to_ptree();
};

class project_list
{
    const std::string cis_projects_path_;
    std::vector<project> projects_;
public:
    project_list(const std::string& path);
    pt::ptree to_ptree();
    std::string to_json_string();
    void fetch();
};
