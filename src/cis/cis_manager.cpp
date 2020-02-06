/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

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
        configuration_manager& config,
        database::database_wrapper& db)
    : ioc_(ioc)
    , config_(config)
    , db_(db)
    , fs_(  *config_.get_entry<std::filesystem::path>("cis_root")
                    / cis::projects,
            4,
            std::chrono::seconds(5))
    , session_manager_(ioc, *config_.get_entry<std::filesystem::path>("cis_root")
                   / cis::sessions)
{
    //assert that config is correct
    assert(config_.get_entry<std::filesystem::path>(
                "cis_root") != nullptr);
    assert(config_.get_entry<std::string>(
                "public_address") != nullptr);
    assert(config_.get_entry<uint16_t>(
                "public_port") != nullptr);
    assert(config_.get_entry<std::string>(
                "cis_address") != nullptr);
    assert(config_.get_entry<uint16_t>(
                "cis_port") != nullptr);

    std::ifstream cis_core_conf(
            *config_.get_entry<std::filesystem::path>("cis_root")
                    / core / "cis.conf");

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

        execs_.set(exec_name, exec_file);
    }

    if(!execs_.valid())
    {
        throw load_config_error("Can't load cis.conf");
    }

    refresh_projects();
}

void cis_manager::refresh_projects()
{
    auto tr = db_.make_transaction();

    using namespace sqlite_orm;

    tr->update_all(set(c(&database::project::deleted) = true));

    for(auto it = fs().begin(); it != fs().end(); ++it)
    {
        if(project::is_entry(it))
        {
            auto project_name = it->path().filename().generic_string();

            auto projects = tr->get_all<database::project>(
                    where(c(&database::project::name) == project_name));

            if(projects.size() == 0)
            {
                tr->insert(
                        database::project{
                                -1,
                                project_name});
            }
            else if(projects.size() == 1)
            {
                auto old_project = projects[0];
                old_project.deleted = false;
                tr->update(old_project);
            }
        }
    }

    tr.commit();
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
    return *config_.get_entry<std::filesystem::path>("cis_root");
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

void cis_manager::create_project(
        const std::string& project_name,
        std::error_code& ec)
{
    fs().create_directory(
            std::filesystem::path{"/"} / project_name,
            ec);

    if(ec)
    {
        return;
    }

    try
    {
        using namespace sqlite_orm;

        auto db = db_.make_transaction();

        auto projects = db->get_all<database::project>(
                where(c(&database::project::name) == project_name));

        if(projects.size() == 0)
        {
            db->insert(
                    database::project{
                            -1,
                            project_name});
        }
        else if(projects.size() == 1)
        {
            auto old_project = projects[0];
            old_project.deleted = false;
            db->update(old_project);
        }

        db.commit();
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return;
    }
}

void cis_manager::remove_project(
        const project_info_t& project,
        std::error_code& ec)
{
    auto project_name = project->dir_entry().path().filename().generic_string();
    project->iterator().remove();

    fs().root().invalidate();

    try
    {
        using namespace sqlite_orm;

        auto db = db_.make_transaction();

        db->update_all(
                set(c(&database::project::deleted) = true),
                where(c(&database::project::name) == project_name));

        db.commit();
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return;
    }
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
    auto project_path =
            *config_.get_entry<std::filesystem::path>("cis_root")
                    / cis::projects / project_name;

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
        bool force,
        const std::vector<std::pair<std::string, std::string>>& params,
        std::function<void(const std::string&)> on_session_started,
        std::function<void(const std::string&)> on_session_finished,
        const std::string& username)
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
                    fs(),
                    webui_config{
                            *config_.get_entry<std::string>("public_address"),
                            *config_.get_entry<uint16_t>("public_port"),
                            *config_.get_entry<std::string>("cis_address"),
                            *config_.get_entry<uint16_t>("cis_port")},
                    *config_.get_entry<std::filesystem::path>("cis_root"),
                    execs_.startjob,
                    username),
            project_name,
            job_name,
            force,
            params,
            on_session_started,
            on_session_finished]
            (auto&& continuation) mutable
            {
                job.run(project_name,
                        job_name,
                        force,
                        params,
                        [   &,
                            on_session_started = std::move(on_session_started),
                            job_path = std::filesystem::path{project_name} / job_name
                        ](const std::string& session_id)
                        {
                            if(auto it = fs_.find(job_path);
                                    it != fs_.end())
                            {
                                it.invalidate();
                            }

                            if(on_session_started)
                            {
                                on_session_started(session_id);
                            }
                        },
                        [   &,
                            on_session_finished = std::move(on_session_finished)
                        ](const std::string& session_id)
                        {
                            session_manager_.finish_session(session_id);

                            if(on_session_finished)
                            {
                                on_session_finished(session_id);
                            }
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

                            if(continuation)
                            {
                                continuation(std::forward<decltype(args)>(args)...);
                            }
                        });
            },
            ioc_.get_executor()};
}

cis_manager::add_cron_task_t cis_manager::add_cron(
        const std::string& project_name,
        const std::string& job_name,
        const std::string& cron_expression)
{
    if(get_job_info(project_name, job_name) == nullptr)
    {
        return {
            [](auto&& continuation)
            {
                continuation(false);
            },
            ioc_.get_executor()};
    }

    auto env = boost::this_process::environment();
    env["cis_base_dir"] = canonical(
            *config_.get_entry<std::filesystem::path>("cis_root")).string();

    return {
        [
            cp = std::make_shared<child_process>(
                    ioc_,
                    env,
                    std::filesystem::path{cis::get_root_dir()} / cis::core,
                    execs_.cis_cron),
            project_name,
            job_name,
            cron_expression
        ](auto&& continuation)
        {
            cp->run({"--add", cron_expression, path_cat(project_name, "/" + job_name)},
                    nullptr,
                    std::make_shared<task_callback>(
                        [continuation]()
                        {
                            continuation(true);
                        },
                        [continuation]()
                        {
                            continuation(false);
                        }));
        },
        ioc_.get_executor()};
}

cis_manager::remove_cron_task_t cis_manager::remove_cron(
        const std::string& project_name,
        const std::string& job_name,
        const std::string& cron_expression)
{
    if(get_job_info(project_name, job_name) == nullptr)
    {
        return {
            [](auto&& continuation)
            {
                continuation(false);
            },
            ioc_.get_executor()};
    }

    auto env = boost::this_process::environment();
    env["cis_base_dir"] = canonical(
            *config_.get_entry<std::filesystem::path>("cis_root")).string();

    return {
        [
            cp = std::make_shared<child_process>(
                    ioc_,
                    env,
                    std::filesystem::path{cis::get_root_dir()} / cis::core,
                    execs_.cis_cron),
            project_name,
            job_name,
            cron_expression
        ](auto&& continuation)
        {
            cp->run({"--del", cron_expression, path_cat(project_name, "/" + job_name)},
                    nullptr,
                    std::make_shared<task_callback>(
                        [continuation]()
                        {
                            continuation(true);
                        },
                        [continuation]()
                        {
                            continuation(false);
                        }));
        },
        ioc_.get_executor()};
}

cis_manager::list_cron_task_t cis_manager::list_cron(const std::string& mask)
{
    auto env = boost::this_process::environment();
    env["cis_base_dir"] = canonical(
                *config_.get_entry<std::filesystem::path>("cis_root")).string();

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
                auto entries = std::make_shared<std::vector<cron_entry>>();

                auto reader = std::make_shared<buffer_reader>(
                        [&, entries](const std::vector<char>& buf) mutable
                        {
                            parse_cron_list(buf, *entries);
                        });

                cp->run({"--list", mask},
                        std::make_shared<process_io_handler>(
                                [](auto){},
                                [reader = std::move(reader)](auto pipe)
                                {
                                    reader->accept_pipe(pipe);
                                },
                                [](auto){}),
                        std::make_shared<task_callback>(
                            [continuation, entries]()
                            {
                                continuation(true, *entries);
                            },
                            [continuation]()
                            {
                                continuation(false, {});
                            }));
            },
            ioc_.get_executor()};
}

std::shared_ptr<session_interface> cis_manager::connect_to_session(
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

void cis_manager::parse_cron_list(
        const std::vector<char>& exe_output,
        std::vector<cron_entry>& entries)
{
    auto it = exe_output.begin();
    while(it != exe_output.end())
    {
        auto it1 = std::find(it, exe_output.end(), '/');
        auto it2 = std::find(it1, exe_output.end(), ' ');
        auto it3 = std::find(it2, exe_output.end(), '\n');

        //[it, it1)/(it1, it2) (it2, it3)\n

        if(it1 == exe_output.end() || it2 == exe_output.end())
        {
            return;
        }

        if(std::distance(it, it1) == 0
        || std::distance(it1 + 1, it2) == 0
        || std::distance(it2 + 1, it3) == 0)
        {
            return;
        }

        std::string project(it, it1);
        std::string job(it1 + 1, it2);
        std::string expr(it2 + 1, it3);

        entries.push_back({project, job, expr});

        if(it3 != exe_output.end())
        {
            ++it3;
        }

        it = it3;
    }
}

} // namespace cis
