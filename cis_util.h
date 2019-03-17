#pragma once

#include <string>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <rapidjson/document.h>

struct project
{
    project(std::string name);
    std::string name;
    rapidjson::Document to_json(
        rapidjson::Document document = {},
        rapidjson::Value value = {});
};

class project_list
{
    const std::string cis_projects_path_;
    std::vector<project> projects_;
public:
    project_list(const std::string& path);
    rapidjson::Document to_json(
        rapidjson::Document document = {},
        rapidjson::Value value = {});
    std::string to_json_string();
    void fetch();
};

void run_job(
        boost::asio::io_context& ctx,
        const std::string& project,
        const std::string& name);
