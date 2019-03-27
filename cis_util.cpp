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

subproject::subproject(const std::string& subproject_name)
    : name(subproject_name)
{}

bool subproject::comp::operator()(const subproject& lhs, const subproject& rhs) const
{
    return lhs.name < rhs.name;
}

bool subproject::comp::operator()(const subproject& lhs, const std::string& rhs) const
{
    return lhs.name < rhs;
}

bool subproject::comp::operator()(const std::string& lhs, const subproject& rhs) const
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

project_list::project_list()
    : cis_projects_path_(path_cat(cis::get_root_dir(), cis::projects))
{
    fetch();
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
                for(auto& subproject: fs::directory_iterator(project))
                {
                    if(subproject.is_directory())
                    {
                        auto [subproject_it, result] = project_it->second.emplace(
                            std::piecewise_construct,
                            std::make_tuple(subproject.path().filename()),
                            std::make_tuple());
                        if(result == false)
                        {
                            continue; //FIXME
                        }
                        for(auto& build: fs::directory_iterator(subproject))
                        {
                            if(build.is_directory())
                            {
                                std::ifstream exitcode_file(path_cat(build.path().c_str(), "/exitcode.txt"));
                                int exitcode;
                                exitcode_file >> exitcode;
                                std::ifstream output_file(path_cat(build.path().c_str(), "/output.txt"));
                                std::string date;
                                output_file >> date;
                                subproject_it->second.emplace(
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
        const subproject& s,
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
