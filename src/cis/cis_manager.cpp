#include "cis/cis_manager.h"

#include <iostream>

#include <boost/process.hpp>
#include <boost/asio/system_executor.hpp>

#include "exceptions/load_config_error.h"
#include "child_process.h"
#include "cis/dirs.h"
#include "file_util.h"

namespace cis
{

cis_manager::cis_manager(
        boost::asio::io_context& ioc,
        std::filesystem::path cis_root,
        database::database_wrapper& db)
    : ioc_(ioc)
    , cis_root_(std::move(cis_root))
    , projects_(db)
    , fs_(cis_root_ / cis::projects, &projects_)
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
        it->update();
        return true;
    }
    return false;
}

bool cis_manager::remove(const std::filesystem::path& path)
{
    if(auto it = fs_.find(path);
            it != fs_.end())
    {
        it->remove();
        return true;
    }
    return false;
}

std::filesystem::path cis_manager::get_projects_path() const
{
    return cis_root_;
}

fs_cache<fs_mapper>& cis_manager::fs()
{
    return fs_;
}

immutable_container_proxy<
            std::map<std::string, project>> cis_manager::get_projects()
{
    return projects_.get_projects();
}

const project* cis_manager::get_project_info(
        const std::string& project_name) const
{
    auto& projects = projects_.get_projects();
    if(auto project_it = projects.find(project_name);
            project_it != projects.cend())
    {
        return &(project_it->second);
    }
    return nullptr;
}

const job* cis_manager::get_job_info(
        const std::string& project_name,
        const std::string& job_name) const
{
    auto* project = get_project_info(project_name);
    if(project != nullptr)
    {
        auto& jobs = project->get_jobs();
        if(auto job_it = jobs.find(job_name);
                job_it != jobs.cend())
        {
            return &(job_it->second);
        }
    }
    return nullptr;
}

const build* cis_manager::get_build_info(
        const std::string& project_name,
        const std::string& job_name,
        const std::string& build_name) const
{
    auto* job = get_job_info(project_name, job_name);
    if(job != nullptr)
    {
        auto& builds = job->get_builds();
        if(auto build_it = builds.find(build_name);
                build_it != builds.cend())
        {
            return &(build_it->second);
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

bool cis_manager::run_job(
        const std::string& project_name,
        const std::string& job_name,
        const std::vector<std::string>& params)
{
    if(get_job_info(project_name, job_name) == nullptr)
    {
        return false;
    }
    auto executable = canonical(cis_root_ / core / execs_.startjob);
    auto env = boost::this_process::environment();
    env["cis_base_dir"] = canonical(cis_root_).string();
    auto cp = std::make_shared<child_process>(ioc_, env);
    cp->set_interactive_params(params);
    cp->run(executable.string(),
            {path_cat(project_name, "/" + job_name)},
            [project_name, job_name](
                    int exit,
                    const std::vector<char>&)
            {
                std::cout << "job " << project_name
                          << "/" << job_name 
                          << " finished" << std::endl;
                std::cout << "process exited with " << exit << std::endl;
            },
            true);
    return true;
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
    auto executable = canonical(cis_root_ / core / execs_.cis_cron);
    auto env = boost::this_process::environment();
    env["cis_base_dir"] = canonical(cis_root_).string();
    auto cp = std::make_shared<child_process>(ioc_, env);
    cp->run(executable.string(),
            {"--add", cron_expression, path_cat(project_name, "/" + job_name)},
            {});
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
    auto executable = canonical(cis_root_ / core / execs_.cis_cron);
    auto env = boost::this_process::environment();
    env["cis_base_dir"] = canonical(cis_root_).string();
    auto cp = std::make_shared<child_process>(ioc_, env);
    cp->run(executable.string(),
            {"--del", cron_expression, path_cat(project_name, "/" + job_name)},
            {});
    return true;
}

cis_manager::list_cron_task_t cis_manager::list_cron(const std::string& mask)
{
    auto executable = canonical(cis_root_ / core / execs_.cis_cron);
    auto env = boost::this_process::environment();
    env["cis_base_dir"] = canonical(cis_root_).string();

    return {[&, cp = std::make_shared<child_process>(ioc_, env),
                executable = executable.string(),
                mask](auto&& continuation)
            {
                cp->run(executable,
                        {"--list", mask},
                        [&, continuation](
                            int exit_code,
                            const std::vector<char>& buf) mutable
                        {
                            std::vector<cron_entry> entries;
                            parse_cron_list(buf, entries);
                            continuation(entries);
                        });
            },
            boost::asio::system_executor{}};
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
