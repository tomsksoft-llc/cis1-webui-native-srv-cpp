#include "cis_util.h"

#include <filesystem>
#include <iostream>

#include <boost/process.hpp>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "child_process.h"
#include "dirs.h"

#include <unistd.h>

namespace fs = std::filesystem;

project::project(std::string n)
    : name(std::move(n))
{}

rapidjson::Document project::to_json(
        rapidjson::Document document,
        rapidjson::Value value)
{
    document.SetObject();
    value.SetString(name.c_str(), name.length(), document.GetAllocator());
    document.AddMember("name", value, document.GetAllocator());
    value.SetArray();
    document.AddMember("subprojects", value, document.GetAllocator());

    return document;
}

project_list::project_list(const std::string& path)
    : cis_projects_path_(path)
{
}

void project_list::fetch()
{
    projects_.clear();
    try
    {
        for(auto& p: fs::directory_iterator(cis_projects_path_))
        {
            if(p.is_directory())
            {
                projects_.emplace_back(p.path().filename().string());
            }
        }
    }
    catch(std::filesystem::filesystem_error& err)
    {}
}

rapidjson::Document project_list::to_json(
        rapidjson::Document document,
        rapidjson::Value value)
{
    document.SetArray();
    rapidjson::Value array_value;
    for(auto& project : projects_)
    {
        array_value.CopyFrom(project.to_json(), document.GetAllocator());
        document.PushBack(array_value, document.GetAllocator());
    }

    return document;
}

std::string project_list::to_json_string()
{
    auto document = to_json();
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer.GetString();
}

void run_job(
        boost::asio::io_context& ctx,
        const std::string& project,
        const std::string& name)
{
    auto env = boost::this_process::environment();
    env["cis_base_dir"] = cis::get_root_dir();
    std::make_shared<child_process>(ctx, env)->run(
            "sh",
            {"startjob", project + "/" + name},
            [](int exit, std::vector<char>& buffer, const std::error_code& ec)
            {
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
            });
}
