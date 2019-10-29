#include "cis/cis_manager.h"

#include <iostream>

#include <boost/process.hpp>
#include <boost/asio/system_executor.hpp>

#include "cis/cis_job.h"
#include "exceptions/load_config_error.h"
#include "child_process.h"
#include "cis/dirs.h"
#include "file_util.h"
#include "cis/cis_structs.h"

namespace cis
{

cis_manager::cis_manager(
        boost::asio::io_context& ioc,
        std::filesystem::path cis_root,
        boost::asio::ip::address webui_address,
        unsigned short webui_port,
        database::database_wrapper& db)
    : ioc_(ioc)
    , cis_root_(std::move(cis_root))
    , webui_address_(webui_address)
    , webui_port_(webui_port)
    , fs_(cis_root_ / cis::projects, 4, std::chrono::seconds(5))
    , session_manager_(ioc)
{
    std::ifstream cis_core_conf(cis_root_ / core / "cis.conf");

    while(cis_core_conf.good())
    {
        std::string exec_name;
        std::string exec_file;
        std::getline(cis_core_conf, exec_name, '=');
        std::getline(cis_core_conf, exec_file, '\n');
        if(exec_name.empty() || exec_file.empty())
        {
            break;
        }

        if(!execs_.set(exec_name, exec_file))
        {
            throw load_config_error("Can't load cis.conf");
        }
    }

    if(!execs_.valid())
    {
        throw load_config_error("Can't load cis.conf");
    }
}

bool cis_manager::refresh(const std::filesystem::path& path)
{
    if(auto it = fs_.find(path);
            it != fs_.end())
    {
        it.update();

        return true;
    }

    return false;
}

bool cis_manager::remove(const std::filesystem::path& path)
{
    if(auto it = fs_.find(path);
            it != fs_.end())
    {
        it.remove();

        return true;
    }

    return false;
}

std::filesystem::path cis_manager::get_projects_path() const
{
    return cis_root_;
}

fs_cache& cis_manager::fs()
{
    return fs_;
}

cis_manager::project_list_t cis_manager::get_project_list()
{
    project_list_t list;

    for(auto it = fs().begin(); it != fs().end(); ++it)
    {
        if(project::is_entry(it))
        {
            list.push_back(
                    static_cast<std::shared_ptr<project_interface>>(
                            std::make_shared<project>(it)));
        }
        else
        {
            list.push_back(std::make_shared<fs_entry_ref>(it));
        }
    }

    return list;
}

cis_manager::project_info_t cis_manager::get_project_info(
        const std::string& project_name)
{
    auto it = fs().find(std::filesystem::path{"/" + project_name});

    if(it != fs().end() && project::is_entry(it))
    {
        return std::make_shared<project>(it);
    }

    return nullptr;
}

cis_manager::job_info_t cis_manager::get_job_info(
        const std::string& project_name,
        const std::string& job_name)
{
    auto project = get_project_info(project_name);

    if(project != nullptr)
    {
        auto job = project->get_job_info(job_name);

        if(job != nullptr)
        {
            return job;
        }
    }

    return nullptr;
}

cis_manager::build_info_t cis_manager::get_build_info(
        const std::string& project_name,
        const std::string& job_name,
        const std::string& build_name)
{
    auto job = get_job_info(project_name, job_name);

    if(job != nullptr)
    {
        auto build = job->get_build_info(build_name);

        if(build != nullptr)
        {
            return build;
        }
    }

    return nullptr;
}

bool cis_manager::rename_job(
        const std::string& project_name,
        const std::string& job_name,
        const std::string& new_name)
{
    auto project_path = cis_root_ / cis::projects / project_name;

    std::error_code ec;
    std::filesystem::rename(
            project_path / job_name,
            project_path / new_name,
            ec);

    return !ec;
}

cis_manager::run_job_task_t cis_manager::run_job(
        const std::string& project_name,
        const std::string& job_name,
        const std::vector<std::string>& params,
        std::function<void(const std::string&)> on_session_started,
        std::function<void(const std::string&)> on_session_finished)
{
    if(get_job_info(project_name, job_name) == nullptr)
    {
        return {
            [](auto&& continuation)
            {
                continuation({false, -1, std::nullopt});
            },
            ioc_.get_executor()};
    }

    return {[&,
            job = cis_job(
                    ioc_,
                    webui_address_,
                    webui_port_,
                    cis_root_,
                    execs_.startjob),
            project_name,
            job_name,
            params,
            on_session_started,
            on_session_finished]
            (auto&& continuation) mutable
            {
                job.run(project_name,
                        job_name,
                        params,
                        on_session_started,
                        [&](const std::string& session_id)
                        {
                            session_manager_.finish_session(session_id);

                            on_session_finished(session_id);
                        },
                        [&,
                         continuation,
                         project_name,
                         job_name](auto&&... args)
                        {
                            auto job = get_job_info(project_name, job_name);
                            if(job != nullptr)
                            {
                                job->invalidate();

                            }

                            continuation(std::forward<decltype(args)>(args)...);
                        });
            },
            ioc_.get_executor()};
}

bool cis_manager::add_cron(
        const std::string& project_name,
        const std::string& job_name,
        const std::string& cron_expression)
{
    if(get_job_info(project_name, job_name) == nullptr)
    {
        return false;
    }

    auto env = boost::this_process::environment();
    env["cis_base_dir"] = canonical(cis_root_).string();

    auto cp = std::make_shared<child_process>(
            ioc_,
            env,
            std::filesystem::path{cis::get_root_dir()} / cis::core,
            execs_.cis_cron);

    cp->run({"--add", cron_expression, path_cat(project_name, "/" + job_name)},
            nullptr,
            nullptr);

    return true;
}

bool cis_manager::remove_cron(
        const std::string& project_name,
        const std::string& job_name,
        const std::string& cron_expression)
{
    if(get_job_info(project_name, job_name) == nullptr)
    {
        return false;
    }

    auto env = boost::this_process::environment();
    env["cis_base_dir"] = canonical(cis_root_).string();

    auto cp = std::make_shared<child_process>(
            ioc_,
            env,
            std::filesystem::path{cis::get_root_dir()} / cis::core,
            execs_.cis_cron);

    cp->run({"--del", cron_expression, path_cat(project_name, "/" + job_name)},
            nullptr,
            nullptr);

    return true;
}

cis_manager::list_cron_task_t cis_manager::list_cron(const std::string& mask)
{
    auto env = boost::this_process::environment();
    env["cis_base_dir"] = canonical(cis_root_).string();

    return {[
            &,
            cp = std::make_shared<child_process>(
                    ioc_,
                    env,
                    std::filesystem::path{cis::get_root_dir()} / cis::core,
                    execs_.cis_cron),
            mask
            ](auto&& continuation)
            {
                auto reader = std::make_shared<buffer_reader>(
                        [&, continuation](const std::vector<char>& buf) mutable
                        {
                            std::vector<cron_entry> entries;
                            parse_cron_list(buf, entries);
                            continuation(entries);
                        });

                cp->run({"--list", mask},
                        std::make_shared<process_io_handler>(
                                [](auto){},
                                [reader = std::move(reader)](auto pipe)
                                {
                                    reader->accept_pipe(pipe);
                                },
                                [](auto){}),
                        nullptr);
            },
            ioc_.get_executor()};
}

std::shared_ptr<session> cis_manager::connect_to_session(
        const std::string& session_id)
{
    return session_manager_.connect(session_id);
}

void cis_manager::subscribe_to_session(
        const std::string& session_id,
        uint64_t ws_session_id,
        std::shared_ptr<subscriber_interface> subscriber)
{
    return session_manager_.subscribe(session_id, ws_session_id, subscriber);
}

std::shared_ptr<subscriber_interface> cis_manager::get_session_subscriber(
        const std::string& session_id,
        uint64_t ws_session_id)
{
    return session_manager_.get_subscriber(session_id, ws_session_id);
}

bool cis_manager::executables::set(
        const std::string& name,
        const std::string& value)
{
    if(name == "startjob")
    {
        startjob = value;
        return true;
    }
    if(name == "setparam")
    {
        setparam = value;
        return true;
    }
    if(name == "getparam")
    {
        getparam = value;
        return true;
    }
    if(name == "setvalue")
    {
        setvalue = value;
        return true;
    }
    if(name == "getvalue")
    {
        getvalue = value;
        return true;
    }
    if(name == "cis_cron")
    {
        cis_cron = value;
        return true;
    }

    return false;
}

bool cis_manager::executables::valid()
{
    return (!startjob.empty()
        &&  !setparam.empty()
        &&  !getparam.empty()
        &&  !setvalue.empty()
        &&  !getvalue.empty()
        &&  !cis_cron.empty());
}

const std::regex cron_line_regex(R"rx(\t((?:\*|(?:[0-9]|1[0-9]|2[0-9]|3[0-9]|4[0-9]|5[0-9])|\*\/(?:[0-9]|1[0-9]|2[0-9]|3[0-9]|4[0-9]|5[0-9])) (?:\*|(?:[0-9]|1[0-9]|2[0-3])|\*\/(?:[0-9]|1[0-9]|2[0-3])) (?:\*|([1-9]|1[0-9]|2[0-9]|3[0-1])|\*\/(?:[1-9]|1[0-9]|2[0-9]|3[0-1])) (?:\*|([1-9]|1[0-2])|\*\/(?:[1-9]|1[0-2])) (?:\*|(?:[0-6])|\*\/(?:[0-6])))\t\/([^\/]+)\/([^\/\n]+)(?:\n|$))rx");

void cis_manager::parse_cron_list(
        const std::vector<char>& exe_output,
        std::vector<cron_entry>& entries)
{
    auto start = exe_output.cbegin();
    auto end = exe_output.cend();
    std::match_results<std::vector<char>::const_iterator> what;
    auto flags = std::regex_constants::match_default;

    while(regex_search(start, end, what, cron_line_regex, flags))
    {
        entries.push_back({what[4], what[5], what[1]});
        start = what[0].second;
        flags |= std::regex_constants::match_prev_avail;
    }
}

} // namespace cis
