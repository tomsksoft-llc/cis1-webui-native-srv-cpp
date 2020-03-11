/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>
#include <functional>

#include <boost/asio.hpp>

#include "child_process.h"
#include "fs_cache.h"

namespace cis
{

struct execution_info
{
    bool success;
    std::optional<int> exit_code;
    std::optional<std::string> exit_message;
    std::optional<std::string> session_id;
};

struct webui_config
{
    std::string public_address;
    uint16_t public_port;

    std::string internal_address;
    uint16_t internal_port;
};

class cis_job
{
public:
    cis_job(boost::asio::io_context& ioc,
            fs_cache& fs,
            const webui_config& webui,
            const std::filesystem::path& cis_root,
            const std::string& startjob_exec,
            const std::string& email);

    void run(
            const std::string& project_name,
            const std::string& job_name,
            bool force,
            const std::vector<std::pair<std::string, std::string>>& params,
            std::function<
                    void(const std::string&)> on_session_started,
            std::function<
                    void(const std::string&)> on_session_finished,
            std::function<
                    void(const execution_info&)> on_job_finished);

    std::optional<std::string> session_id();

private:
    fs_cache& fs_;
    std::shared_ptr<line_reader> reader_ = nullptr;
    std::shared_ptr<async_buffer_writer> writer_ = nullptr;
    std::shared_ptr<child_process> cp_ = nullptr;
    std::optional<std::string> project_job_;
    std::optional<std::string> session_id_;
    std::optional<std::string> build_;
    std::function<void(const std::string&)> session_started_cb_;
    std::function<void(const std::string&)> session_finished_cb_;
    std::function<void(const execution_info&)> job_finished_cb_;

    void on_session_started(const std::string& session_id);
    void on_job_finished();
    void on_job_failed();
};

} // namespace cis
