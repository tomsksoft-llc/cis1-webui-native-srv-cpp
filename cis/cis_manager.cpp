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
    , cis_root_(cis_root)
    , projects_(ioc_, db)
{
    projects_.run();
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

std::filesystem::path cis_manager::get_projects_path() const
{
    return cis_root_;
}

const project::map_t& cis_manager::get_projects() const
{
    return projects_.get();
}

const project_info* const cis_manager::get_project_info(
        const std::string& project_name) const
{
    auto project_it = projects_.get().find(project_name);
    if(project_it != projects_.get().cend())
    {
        return &(project_it->second);
    }
    return nullptr;
}

const job_info* const cis_manager::get_job_info(
        const std::string& project_name,
        const std::string& job_name) const
{
    auto* project = get_project_info(project_name);
    if(project != nullptr)
    {
        auto job_it = project->jobs.find(job_name);
        if(job_it != project->jobs.cend())
        {
            return &(job_it->second);
        }
    }
    return nullptr;
}

const build* const cis_manager::get_build_info(
        const std::string& project_name,
        const std::string& job_name,
        const std::string& build_name) const
{
    auto* job = get_job_info(project_name, job_name);
    if(job != nullptr)
    {
        auto build_it = job->builds.find(build_name);
        if(build_it != job->builds.cend())
        {
            return &(*build_it);
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
    if(!get_job_info(project_name, job_name))
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
    if(     !startjob.empty()
        &&  !setparam.empty()
        &&  !getparam.empty()
        &&  !setvalue.empty()
        &&  !getvalue.empty())
    {
        return true;
    }
    return false;
}

} // namespace cis
