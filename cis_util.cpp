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
        const std::string& build_date)
    : name(build_name)
    , status(build_status)
    , date(build_date)
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

void project_list::fetch()
{
    projects.clear();
    try
    {
        for(auto& project: fs::directory_iterator(cis_projects_path_))
        {
            if(project.is_directory())
            {
                auto [project_it, result] = projects.emplace(
                        std::piecewise_construct,
                        std::make_tuple(project.path().filename()),
                        std::make_tuple());
                if(result == false)
                {
                    continue; //FIXME
                }
                for(auto& job: fs::directory_iterator(project))
                {
                    if(job.is_directory())
                    {
                        auto [job_it, result] = project_it->second.emplace(
                            std::piecewise_construct,
                            std::make_tuple(job.path().filename()),
                            std::make_tuple());
                        if(result == false)
                        {
                            continue; //FIXME
                        }
                        for(auto& job_file: fs::directory_iterator(job))
                        {
                            if(job_file.is_directory())
                            {
                                auto& build = job_file;
                                std::ifstream exitcode_file(path_cat(build.path().c_str(), "/exitcode.txt"));
                                int exitcode;
                                exitcode_file >> exitcode;
                                std::ifstream output_file(path_cat(build.path().c_str(), "/output.txt"));
                                std::string date;
                                output_file >> date;
                                job_it->second.builds.emplace(
                                        build.path().filename(),
                                        exitcode,
                                        date.substr(0, 10));
                            }
                            else if(job_file.is_regular_file())
                            {
                                if(job_file.path().filename() == "params")
                                {
                                    //TODO prevent crash on invalid params file
                                    auto& job_params = job_it->second.params;
                                    std::ifstream params_file(job_file.path()); 
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
                                job_files.emplace_back(job_file.path().filename());
                            }
                        }
                    }
                }
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
