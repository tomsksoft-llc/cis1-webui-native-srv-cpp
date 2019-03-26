#pragma once

#include <string>
#include <map>
#include <set>
#include <filesystem>

#include <boost/asio/io_context.hpp>
#include <rapidjson/document.h>

struct build
{
    explicit build(
            const std::string& build_name,
            const std::string& build_status,
            const std::string& build_date);
    std::string name;
    std::string status;
    std::string date;
    struct comp
    {
        typedef std::true_type is_transparent;

        bool operator()(const build& lhs, const build& rhs) const;
        bool operator()(const build& lhs, const std::string& rhs) const;
        bool operator()(const std::string& lhs, const build& rhs) const;
    };
    using set_t = std::set<build, comp>;
};

struct subproject
{
    explicit subproject(
            const std::string& subproject_name);

    std::string name;
    struct comp
    {
        typedef std::true_type is_transparent;

        bool operator()(const subproject& lhs, const subproject& rhs) const;
        bool operator()(const subproject& lhs, const std::string& rhs) const;
        bool operator()(const std::string& lhs, const subproject& rhs) const;
    };
    using map_t = std::map<subproject, build::set_t, comp>;
};


struct project
{
    explicit project(
            const std::string& project_name);

    std::string name;
    struct comp
    {
        typedef std::true_type is_transparent;

        bool operator()(const project& lhs, const project& rhs) const;
        bool operator()(const project& lhs, const std::string& rhs) const;
        bool operator()(const std::string& lhs, const project& rhs) const;
    };
    using map_t = std::map<project, subproject::map_t, comp>;
};

class project_list
{
    std::filesystem::path cis_projects_path_;
public:
    project_list();
    project::map_t projects;
    void fetch();
};

rapidjson::Document to_json(
        const project& p,
        rapidjson::Document document = {},
        rapidjson::Value value = {});
rapidjson::Document to_json(
        const subproject& s,
        rapidjson::Document document = {},
        rapidjson::Value value = {});
rapidjson::Document to_json(
        const build& b,
        rapidjson::Document document = {},
        rapidjson::Value value = {});

void run_job(
        boost::asio::io_context& ctx,
        const std::string& project,
        const std::string& name);
