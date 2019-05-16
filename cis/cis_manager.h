#pragma once

#include <string>
#include <filesystem>
#include <map>
#include <vector>

#include <boost/asio.hpp>

#include "database.h"
#include "fs_cache.h"
#include "cis_structs.h"
#include "fs_mapper.h"
#include "immutable_container_proxy.h"

namespace cis
{

class cis_manager
{
public:
    cis_manager(
            boost::asio::io_context& ioc,
            std::filesystem::path cis_root,
            database::database& db);
    cis_manager(const cis_manager&) = delete;

    bool refresh(const std::filesystem::path& path);
    bool remove(const std::filesystem::path& path);
    std::filesystem::path get_projects_path() const;

    fs_cache<fs_mapper>& fs();

    immutable_container_proxy<
            std::map<std::string, project>> get_projects();
    const project* get_project_info(
            const std::string& project_name) const;
    const job* get_job_info(
            const std::string& project_name,
            const std::string& job_name) const;
    const build* get_build_info(
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
private:
    struct executables
    {
        std::string startjob;
        std::string setparam;
        std::string getparam;
        std::string setvalue;
        std::string getvalue;
        bool set(const std::string& name, const std::string& value);
        bool valid();
    };
    boost::asio::io_context& ioc_;
    std::filesystem::path cis_root_;
    project_list projects_;
    fs_cache<fs_mapper> fs_;
    executables execs_;
};

} // namespace cis
