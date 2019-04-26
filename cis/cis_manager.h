#pragma once

#include <string>
#include <filesystem>

#include <boost/asio.hpp>

#include "database.h"
#include "project_list.h"

namespace cis
{

class cis_manager
{
    boost::asio::io_context& ioc_;
    std::filesystem::path cis_root_;
    project_list projects_;
public:
    cis_manager(
            boost::asio::io_context& ioc,
            std::filesystem::path cis_root,
            database::database& db);
    cis_manager(const cis_manager&) = delete;

    std::filesystem::path get_projects_path() const;

    const project::map_t& get_projects() const;
    const project_info* const get_project_info(
            const std::string& project_name) const;
    const job_info* const get_job_info(
            const std::string& project_name,
            const std::string& job_name) const;
    const build* const get_build_info(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& build_name) const;

    bool rename_job(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& new_name);
    bool run_job(
            const std::string& project_name,
            const std::string& job_name,
            const std::vector<std::string>& params = {});

};

} // namespace cis
