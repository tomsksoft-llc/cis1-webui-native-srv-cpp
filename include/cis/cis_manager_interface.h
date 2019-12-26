/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>
#include <filesystem>
#include <map>
#include <vector>

#include <boost/asio.hpp>
#include <cis1_proto_utils/transaction.h>

#include "cis_job.h"
#include "database.h"
#include "fs_cache.h"
#include "cis_structs_interface.h"
#include "immutable_container_proxy.h"
#include "bound_task_chain.h"
#include "session.h"

namespace cis
{

struct cron_entry
{
    std::string project;
    std::string job;
    std::string cron_expr;
};

struct cis_manager_interface
{
    using list_cron_task_t = typename make_async_task_t<
            void(bool, const std::vector<cron_entry>&)>::task;

    using add_cron_task_t = typename make_async_task_t<
            void(bool)>::task;

    using remove_cron_task_t = typename make_async_task_t<
            void(bool)>::task;

    using run_job_task_t = typename make_async_task_t<
            void(const execution_info&)>::task;

    virtual ~cis_manager_interface() = default;

    virtual void refresh_projects() = 0;

    virtual bool refresh(const std::filesystem::path& path) = 0;

    virtual bool remove(const std::filesystem::path& path) = 0;

    virtual std::filesystem::path get_projects_path() const = 0;

    virtual fs_cache& fs() = 0;

    using project_list_t =
        std::vector<
                std::variant<
                        std::shared_ptr<fs_entry_interface>,
                        std::shared_ptr<project_interface>>>;

    virtual project_list_t get_project_list() = 0;

    using project_info_t = std::shared_ptr<project_interface>;

    virtual void create_project(
            const std::string& project_name,
            std::error_code& ec) = 0;

    virtual void remove_project(
            const project_info_t& project,
            std::error_code& ec) = 0;

    virtual project_info_t get_project_info(
            const std::string& project_name) = 0;

    using job_info_t = std::shared_ptr<job_interface>;

    virtual job_info_t get_job_info(
            const std::string& project_name,
            const std::string& job_name) = 0;

    using build_info_t = std::shared_ptr<build_interface>;

    virtual build_info_t get_build_info(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& build_name) = 0;

    virtual bool rename_job(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& new_name) = 0;

    virtual run_job_task_t run_job(
            const std::string& project_name,
            const std::string& job_name,
            bool force,
            const std::vector<std::string>& params,
            std::function<
                    void(const std::string&)> on_session_started,
            std::function<
                    void(const std::string&)> on_session_finished,
            const std::string& username) = 0;

    virtual add_cron_task_t add_cron(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& cron_expression) = 0;

    virtual remove_cron_task_t remove_cron(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& cron_expression) = 0;

    virtual list_cron_task_t list_cron(
            const std::string& mask) = 0;

    virtual std::shared_ptr<session_interface> connect_to_session(
            const std::string& session_id) = 0;

    virtual void subscribe_to_session(
            const std::string& session_id,
            uint64_t ws_session_id,
            std::shared_ptr<subscriber_interface> subscriber) = 0;

    virtual std::shared_ptr<subscriber_interface> get_session_subscriber(
            const std::string& session_id,
            uint64_t ws_session_id) = 0;
};

} // namespace cis
