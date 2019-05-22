#include "cis_manager.h"

#include <iostream>

#include <boost/process.hpp>

#include "exceptions/load_config_error.h"
#include "child_process.h"
#include "dirs.h"
#include "file_util.h"

namespace cis
{

cis_manager::cis_manager(
        boost::asio::io_context& ioc,
        std::filesystem::path cis_root,
        database::database& db)
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

    if(!ec)
    {
        return true;
    }

    return false;
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
    env["cis_base_dir"] = canonical(cis_root_);
    auto cp = std::make_shared<child_process>(ioc_, env);
    cp->set_interactive_params(params);
    cp->run(executable.string(),
            {path_cat(project_name, "/" + job_name)},
            [project_name, job_name](
                    int exit,
                    std::vector<char>& buffer,
                    const std::error_code& ec)
            {
                std::cout << "job " + project_name
                        + "/" + job_name + " finished" << std::endl;
                if(!ec)
                {
                    std::cout << "process exited with " << exit << std::endl;
                }
                else
                {
                    std::cout << "error" << std::endl;
                }
                //TODO update builds (emit cis_updated or something)
            });
    return true;
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

    return false;
}

bool cis_manager::executables::valid()
{
    return (!startjob.empty()
        &&  !setparam.empty()
        &&  !getparam.empty()
        &&  !setvalue.empty()
        &&  !getvalue.empty());
}

} // namespace cis