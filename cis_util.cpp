#include "cis_util.h"

#include <filesystem>
#include <iostream>
#include <fstream>

#include <boost/process.hpp>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "child_process.h"
#include "dirs.h"
#include "file_util.h"

#include <unistd.h>

namespace fs = std::filesystem;

build::build(
        const std::string& build_name,
        int build_status,
        const std::string& build_date,
        std::vector<std::string> build_artifacts)
    : name(build_name)
    , status(build_status)
    , date(build_date)
    , artifacts(std::move(build_artifacts))
{}
param::param(
        const std::string& param_name,
        const std::string& param_default_value)
    : name(param_name)
    , default_value(param_default_value)
{}
job::job(const std::string& job_name)
    : name(job_name)
{}


project::project(const std::string& project_name)
        : name(project_name)
{}

project_list::project_list(boost::asio::io_context& ioc)
    : cis_projects_path_(path_cat(cis::get_root_dir(), cis::projects))
    , strand_(ioc.get_executor())
    , timer_(ioc,
            (std::chrono::steady_clock::time_point::max)())
{
    fetch();
}

void project_list::run()
{
    timer_.expires_after(std::chrono::seconds(15));
    timer_.async_wait(
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &project_list::on_timer,
                shared_from_this(),
                std::placeholders::_1)));
}

void project_list::on_timer(boost::system::error_code ec)
{
    if(ec && ec != boost::asio::error::operation_aborted)
    {
        std::cerr << "project_list::on_timer() error" << ": " << ec.message() << "\n";
        return;
    }

    if(timer_.expiry() <= std::chrono::steady_clock::now())
    {
        fetch();
        timer_.expires_after(std::chrono::seconds(15));
    }
    timer_.async_wait(
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &project_list::on_timer,
                shared_from_this(),
                std::placeholders::_1)));
}

void project_list::fetch_build(
        const std::filesystem::directory_entry& build_dir,
        job::map_t::iterator it)
{
    std::ifstream exitcode_file(build_dir.path() / "exitcode.txt");
    int exitcode;
    exitcode_file >> exitcode;
    std::ifstream output_file(build_dir.path() / "output.txt");
    std::string date;
    output_file >> date;
    std::vector<std::string> artifacts;
    for(auto& dir_entry: fs::directory_iterator(build_dir))
    {
        if(dir_entry.is_regular_file())
        {
            artifacts.emplace_back(dir_entry.path().filename().c_str());
        }
    }
    auto& builds = it->second.builds;
    builds.emplace(
            build_dir.path().filename(),
            exitcode,
            date.substr(0, 10),
            std::move(artifacts));
}

void project_list::fetch_job(
        const std::filesystem::directory_entry& job_dir,
        project::map_t::iterator it)
{
    auto [job_it, result] = it->second.jobs.emplace(
        std::piecewise_construct,
        std::make_tuple(job_dir.path().filename()),
        std::make_tuple());
    if(result == false)
    {
        return;
    }
    for(auto& dir_entry: fs::directory_iterator(job_dir))
    {
        if(dir_entry.is_directory())
        {
            fetch_build(dir_entry, job_it);
        }
        else if(dir_entry.is_regular_file())
        {
            if(dir_entry.path().filename() == "params")
            {
                //TODO prevent crash on invalid params file
                auto& job_params = job_it->second.params;
                std::ifstream params_file(dir_entry.path()); 
                while(params_file.good())
                {
                    std::string param;
                    std::string default_value;
                    std::getline(params_file, param, '=');
                    std::getline(params_file, default_value, '\n');
                    if(!default_value.empty())
                    {
                        default_value = default_value.substr(1, default_value.size() - 2);
                    }
                    job_params.emplace(param, default_value);
                }
                
            }
            auto& job_files = job_it->second.files;
            job_files.emplace_back(dir_entry.path().filename());
        }
    }
}

void project_list::fetch_project(
        const std::filesystem::directory_entry& project_dir)
{
    auto [it, result] = projects.emplace(
            std::piecewise_construct,
            std::make_tuple(project_dir.path().filename()),
            std::make_tuple());
    if(result == false)
    {
        return;
    }
    for(auto& dir_entry: fs::directory_iterator(project_dir))
    {
        if(dir_entry.is_directory())
        {
            fetch_job(dir_entry, it);
        }
        else if(dir_entry.is_regular_file())
        {
            auto& project_files = it->second.files;
            project_files.emplace_back(dir_entry.path().filename());
        }
    }
}

void project_list::fetch()
{
    projects.clear();
    try
    {
        for(auto& dir_entry: fs::directory_iterator(cis_projects_path_))
        {
            if(dir_entry.is_directory())
            {
                fetch_project(dir_entry);
            }
        }
    }
    catch(std::filesystem::filesystem_error& err)
    {}
}

void project_list::defer_fetch()
{
    if(timer_.expiry() > std::chrono::steady_clock::now() + std::chrono::seconds(1))
    {
        timer_.expires_after(std::chrono::seconds(1));
    }
}

rapidjson::Document to_json(
        const project& p,
        rapidjson::Document document,
        rapidjson::Value value)
{
    document.SetObject();
    value.SetString(p.name.c_str(), p.name.length(), document.GetAllocator());
    document.AddMember("name", value, document.GetAllocator());
    return document;
}

rapidjson::Document to_json(
        const job& s,
        rapidjson::Document document,
        rapidjson::Value value)
{
    document.SetObject();
    value.SetString(s.name.c_str(), s.name.length(), document.GetAllocator());
    document.AddMember("name", value, document.GetAllocator());
    return document;
}

rapidjson::Document to_json(
        const build& b,
        rapidjson::Document document,
        rapidjson::Value value)
{
    document.SetObject();
    value.SetString(b.name.c_str(), b.name.length(), document.GetAllocator());
    document.AddMember("name", value, document.GetAllocator());
    value.SetInt(b.status);
    document.AddMember("status", value, document.GetAllocator());
    value.SetString(b.date.c_str(), b.date.length(), document.GetAllocator());
    document.AddMember("date", value, document.GetAllocator());
    return document;
}

void run_job(
        boost::asio::io_context& ctx,
        const std::string& project,
        const std::string& name)
{
    auto env = boost::this_process::environment();
    env["cis_base_dir"] = cis::get_root_dir();
    auto cp = std::make_shared<child_process>(ctx, env);
    cp->run(
            "sh",
            {"startjob", path_cat(project, "/" + name)},
            [project, name](int exit, std::vector<char>& buffer, const std::error_code& ec)
            {
                std::cout << "job " + project
                            + "/" + name + " finished" << std::endl;
                if(!ec)
                {
                    std::cout << "process exited with " << exit << std::endl;
                    std::cout << "std_out contain:" << std::endl;
                    std::cout.write(buffer.data(), buffer.size());
                }
                else
                {
                    std::cout << "error" << std::endl;
                }
                //TODO update builds (emit cis_updated or something)
            });
}

void rename_job(
        const std::string& project,
        const std::string& job,
        const std::string& name,
        std::error_code& ec)
{
    auto project_path = path_cat(cis::get_root_dir(), path_cat(cis::projects, "/" + project));
    std::filesystem::rename(path_cat(project_path, "/" + job), path_cat(project_path, "/" + name), ec);
}
