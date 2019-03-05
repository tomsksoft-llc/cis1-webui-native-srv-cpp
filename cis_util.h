#pragma once

#include <string>

#include <boost/property_tree/ptree.hpp>

class job_list
{
    const std::string cis_jobs_;
    boost::property_tree::ptree tree_;
public:
    job_list(const std::string& path);
    std::string to_string();
    void fetch();
};
