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

namespace cis
{

struct execution_info
{
    bool success;
    int exit_code;
    std::optional<std::string> session_id;
};

class cis_job
{
public:
    cis_job(boost::asio::io_context& ioc,
            const boost::asio::ip::address& webui_address,
            short unsigned webui_port,
            const std::filesystem::path& cis_root,
            const std::string& startjob_exec);

    void run(
            const std::string& project_name,
            const std::string& job_name,
            const std::vector<std::string>& params,
            std::function<
                    void(const std::string&)> on_session_started,
            std::function<
                    void(const std::string&)> on_session_finished,
            std::function<
                    void(const execution_info&)> on_job_finished);

    std::optional<std::string> session_id();

private:
    std::shared_ptr<line_reader> reader_ = nullptr;
    std::shared_ptr<async_buffer_writer> writer_ = nullptr;
    std::shared_ptr<child_process> cp_ = nullptr;
    std::optional<std::string> session_id_;
    std::function<void(const std::string&)> session_started_cb_;
    std::function<void(const std::string&)> session_finished_cb_;
    std::function<void(const execution_info&)> job_finished_cb_;

    void on_session_started(const std::string& session_id);
    void on_job_finished();
    void on_job_failed();
};

} // namespace cis