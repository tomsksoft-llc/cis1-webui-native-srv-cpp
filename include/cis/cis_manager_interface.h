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
#include "cis_structs.h"
#include "fs_mapper.h"
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
            void(const std::vector<cron_entry>&)>::task;

    using run_job_task_t = typename make_async_task_t<
            void(const execution_info&)>::task;

    virtual ~cis_manager_interface() = default;

    virtual bool refresh(const std::filesystem::path& path) = 0;

    virtual bool remove(const std::filesystem::path& path) = 0;

    virtual std::filesystem::path get_projects_path() const = 0;

    virtual fs_cache<fs_mapper>& fs() = 0;

    virtual immutable_container_proxy<
            std::map<std::string, project>> get_projects() = 0;

    virtual const project* get_project_info(
            const std::string& project_name) const = 0;

    virtual const job* get_job_info(
            const std::string& project_name,
            const std::string& job_name) const = 0;

    virtual const build* get_build_info(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& build_name) const = 0;

    virtual bool rename_job(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& new_name) = 0;

    virtual run_job_task_t run_job(
            const std::string& project_name,
            const std::string& job_name,
            const std::vector<std::string>& params,
            std::function<
                    void(const std::string&)> on_session_started,
            std::function<
                    void(const std::string&)> on_session_finished) = 0;

    virtual bool add_cron(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& cron_expression) = 0;

    virtual bool remove_cron(
            const std::string& project_name,
            const std::string& job_name,
            const std::string& cron_expression) = 0;

    virtual list_cron_task_t list_cron(
            const std::string& mask) = 0;

    virtual std::shared_ptr<session> connect_to_session(
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
