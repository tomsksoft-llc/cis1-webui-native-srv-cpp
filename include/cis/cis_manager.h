#pragma once

#include <string>
#include <filesystem>
#include <map>
#include <vector>

#include <boost/asio.hpp>

#include "cis_manager_interface.h"
#include "database.h"
#include "fs_cache.h"
#include "session_manager.h"
#include "immutable_container_proxy.h"
#include "bound_task_chain.h"
#include "session.h"

namespace cis
{

class cis_manager
    : public cis_manager_interface
{
public:
    cis_manager(
            boost::asio::io_context& ioc,
            std::filesystem::path cis_root,
            boost::asio::ip::address webui_address,
            unsigned short webui_port,
            database::database_wrapper& db);

    cis_manager(const cis_manager&) = delete;

    bool refresh(const std::filesystem::path& path) override;

    bool remove(const std::filesystem::path& path) override;

    std::filesystem::path get_projects_path() const override;

    fs_cache& fs() override;

    project_list_t get_project_list() override;

    project_info_t get_project_info(
            const std::string& project_name) override;

    job_info_t get_job_info(
            const std::string& project_name,
            const std::string& job_name) override;

    build_info_t get_build_info(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& build_name) override;

    bool rename_job(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& new_name) override;

    run_job_task_t run_job(
            const std::string& project_name,
            const std::string& job_name,
            const std::vector<std::string>& params,
            std::function<
                    void(const std::string&)> on_session_started,
            std::function<
                    void(const std::string&)> on_session_finished) override;

    bool add_cron(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& cron_expression) override;

    bool remove_cron(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& cron_expression) override;

    list_cron_task_t list_cron(
            const std::string& mask) override;

    std::shared_ptr<session> connect_to_session(
            const std::string& session_id) override;

    void subscribe_to_session(
            const std::string& session_id,
            uint64_t ws_session_id,
            std::shared_ptr<subscriber_interface> subscriber) override;

    std::shared_ptr<subscriber_interface> get_session_subscriber(
            const std::string& session_id,
            uint64_t ws_session_id) override;

private:
    struct executables
    {
        std::string startjob;
        std::string setparam;
        std::string getparam;
        std::string setvalue;
        std::string getvalue;
        std::string cis_cron;
        bool set(const std::string& name, const std::string& value);
        bool valid();
    };

    boost::asio::io_context& ioc_;
    std::filesystem::path cis_root_;
    boost::asio::ip::address webui_address_;
    short unsigned webui_port_;
    fs_cache fs_;
    executables execs_;
    session_manager session_manager_;

    void parse_cron_list(
            const std::vector<char>& exe_output,
            std::vector<cron_entry>& entries);
};

} // namespace cis
