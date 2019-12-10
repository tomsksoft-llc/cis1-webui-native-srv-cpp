/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "cis/cis_job.h"

#include <regex>

#include "cis/dirs.h"
#include "file_util.h"

namespace cis
{

cis_job::cis_job(
        boost::asio::io_context& ioc,
        const webui_config& webui,
        const std::filesystem::path& cis_root,
        const std::string& startjob_exec)
{
    auto env = boost::this_process::environment();

    env["cis_base_dir"] = canonical(cis_root).string();
    env["webui_public_address"] = webui.public_address;
    env["webui_public_port"] = std::to_string(webui.public_port);
    env["webui_internal_address"] = webui.internal_address;
    env["webui_internal_port"] = std::to_string(webui.internal_port);

    cp_ = std::make_shared<child_process>(
            ioc,
            env,
            std::filesystem::path{cis::get_root_dir()} / cis::core,
            startjob_exec);
}

void cis_job::run(
        const std::string& project_name,
        const std::string& job_name,
        bool force,
        const std::vector<std::string>& params,
        std::function<
                void(const std::string&)> on_session_started,
        std::function<
                void(const std::string&)> on_session_finished,
        std::function<
                void(const execution_info&)> on_job_finished)
{
    session_started_cb_ = on_session_started;
    session_finished_cb_ = on_session_finished;
    job_finished_cb_ = on_job_finished;

    writer_ = std::make_shared<async_buffer_writer>(
            make_interactive_args_buffer(params));

    reader_ = std::make_shared<line_reader>(
            [
            &,
            got_session_id = false
            ](const std::string& str) mutable
            {
                if(!got_session_id && session_started_cb_)
                {
                    session_started_cb_(str);

                    session_id_ = str;

                    got_session_id = true;
                }
            });

    std::vector<std::string> args;

    args.push_back({path_cat(project_name, "/" + job_name)});

    if(force)
    {
        args.push_back({"--force"});
    }

    cp_->run(args,
            std::make_shared<process_io_handler>(
                    [&](auto&&... args)
                    {
                        writer_->accept_pipe(std::forward<decltype(args)>(args)...);
                    },
                    [&](auto&&... args)
                    {
                        reader_->accept_pipe(std::forward<decltype(args)>(args)...);
                    },
                    [](auto&&... args){}),
            std::make_shared<task_callback>(
                    [&]()
                    {
                        auto& exit_code_line = reader_->last_line();


                        auto exit_code = [&]() -> std::optional<int>
                        {
                            static const std::regex exit_code_regex("Exit code: (\\d+)");
                            if(std::smatch match;
                                    std::regex_match(exit_code_line, match, exit_code_regex))
                            {
                                return std::stoul(match[1]);
                            }

                            return std::nullopt;
                        }();

                        if(session_id_ && session_finished_cb_)
                        {
                            session_finished_cb_(session_id_.value());
                        }

                        if(job_finished_cb_)
                        {
                            job_finished_cb_(
                                    {
                                        true,
                                        exit_code ? *exit_code : -1,
                                        session_id_
                                    });
                        }

                        job_finished_cb_ = {};
                    },
                    [&]()
                    {
                        if(session_id_)
                        {
                            session_finished_cb_(session_id_.value());
                        }

                        job_finished_cb_({false, -1, std::nullopt});

                        job_finished_cb_ = {};
                    }));
}

std::optional<std::string> cis_job::session_id()
{
    return session_id_;
}

} // namespace cis
