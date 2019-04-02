#pragma once

#include <string>
#include <map>
#include <set>
#include <filesystem>
#include <memory>

#include <boost/asio.hpp>
#include <rapidjson/document.h>

template <class T>
struct name_member_comparator
{
    typedef std::true_type is_transparent;

    bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs.name < rhs.name;
    }

    bool operator()(const T& lhs, const std::string& rhs) const
    {
        return lhs.name < rhs;
    }

    bool operator()(const std::string& lhs, const T& rhs) const
    {
        return lhs < rhs.name;
    }
};

struct build
{
    explicit build(
            const std::string& build_name,
            int build_status,
            const std::string& build_date,
            std::vector<std::string> build_artifacts);
    std::string name;
    int status;
    std::string date;
    std::vector<std::string> artifacts;
    using set_t = std::set<build, name_member_comparator<build>>;
};

struct param
{
    explicit param(
            const std::string& param_name,
            const std::string& param_default_value);
    std::string name;
    std::string default_value;
    using map_t = std::set<param, name_member_comparator<param>>;
};

struct job_info
{
    build::set_t builds;
    param::map_t params;
    std::vector<std::string> files;
};

struct job 
{
    explicit job(
            const std::string& job_name);

    std::string name;
    using map_t = std::map<job, job_info, name_member_comparator<job>>;
};


struct project
{
    explicit project(
            const std::string& project_name);

    std::string name;
    using map_t = std::map<project, job::map_t, name_member_comparator<project>>;
};

class project_list
    : public std::enable_shared_from_this<project_list>
{
    std::filesystem::path cis_projects_path_;
    boost::asio::strand<
        boost::asio::io_context::executor_type> strand_;
    boost::asio::steady_timer timer_;

    void on_timer(boost::system::error_code ec);
public:
    project_list(boost::asio::io_context& ioc);
    void run();
    project::map_t projects;
    void fetch();
    void defer_fetch();
};

rapidjson::Document to_json(
        const project& p,
        rapidjson::Document document = {},
        rapidjson::Value value = {});
rapidjson::Document to_json(
        const job& s,
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

void rename_job(
        const std::string& project,
        const std::string& job,
        const std::string& name,
        std::error_code& ec);
