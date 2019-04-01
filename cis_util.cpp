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

bool build::comp::operator()(const build& lhs, const build& rhs) const
{
    return lhs.name < rhs.name;
}

bool build::comp::operator()(const build& lhs, const std::string& rhs) const
{
    return lhs.name < rhs;
}

bool build::comp::operator()(const std::string& lhs, const build& rhs) const
{
    return lhs < rhs.name;
}

job::job(const std::string& job_name)
    : name(job_name)
{}

bool job::comp::operator()(const job& lhs, const job& rhs) const
{
    return lhs.name < rhs.name;
}

bool job::comp::operator()(const job& lhs, const std::string& rhs) const
{
    return lhs.name < rhs;
}

bool job::comp::operator()(const std::string& lhs, const job& rhs) const
{
    return lhs < rhs.name;
}


project::project(const std::string& project_name)
        : name(project_name)
{}

bool project::comp::operator()(const project& lhs, const project& rhs) const
{
    return lhs.name < rhs.name;
}

bool project::comp::operator()(const project& lhs, const std::string& rhs) const
{
    return lhs.name < rhs;
}

bool project::comp::operator()(const std::string& lhs, const project& rhs) const
{
    return lhs < rhs.name;
}

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
                        for(auto& build: fs::directory_iterator(job))
                        {
                            if(build.is_directory())
                            {
                                std::ifstream exitcode_file(path_cat(build.path().c_str(), "/exitcode.txt"));
                                int exitcode;
                                exitcode_file >> exitcode;
                                std::ifstream output_file(path_cat(build.path().c_str(), "/output.txt"));
                                std::string date;
                                output_file >> date;
                                job_it->second.emplace(
                                        build.path().filename(),
                                        exitcode,
                                        date.substr(0, 10));
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
