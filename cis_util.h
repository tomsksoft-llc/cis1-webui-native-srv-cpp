#pragma once

#include <string>
#include <map>
#include <set>
#include <filesystem>
#include <memory>

#include <boost/asio.hpp>
#include <rapidjson/document.h>

struct build
{
    explicit build(
            const std::string& build_name,
            int build_status,
            const std::string& build_date);
    std::string name;
    int status;
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

struct job 
{
    explicit job(
            const std::string& job_name);

    std::string name;
    struct comp
    {
        typedef std::true_type is_transparent;

        bool operator()(const job& lhs, const job& rhs) const;
        bool operator()(const job& lhs, const std::string& rhs) const;
        bool operator()(const std::string& lhs, const job& rhs) const;
    };
    using map_t = std::map<job, build::set_t, comp>;
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
    using map_t = std::map<project, job::map_t, comp>;
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
