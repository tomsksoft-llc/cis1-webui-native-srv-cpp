#include "websocket_event_handlers.h"

#include <chrono>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "websocket_event_list.h"

using namespace std::string_literals;

std::optional<std::string> get_string(
        const rapidjson::Value& value,
        const char* name)
{
    if(value.HasMember(name) && value[name].IsString())
    {
        return value[name].GetString();
    }
    return std::nullopt;
}

std::optional<std::string> ws_handle_authenticate(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto login = get_string(request_data, "login");
    auto pass = get_string(request_data, "pass");
    
    if(!login || !pass)
    {
        return "Invalid JSON.";
    }

    std::string token = authentication_handler->authenticate(login.value(), pass.value());
    
    if(!token.empty())
    {
        ctx.username = login.value();
        ctx.active_token = token;
        response_data.AddMember(
                "token",
                rapidjson::Value().SetString(
                    token.c_str(),
                    token.length(),
                    allocator),
                allocator);
        return std::nullopt;
    }
    else
    {
        return "Wrong username or password.";
    }
}

std::optional<std::string> ws_handle_token(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto token = get_string(request_data, "token");
    if(!token)
    {
        return "Invalid JSON.";
    }

    std::string username = authentication_handler->authenticate(token.value());
    
    if(!username.empty())
    {
        ctx.username = username;
        ctx.active_token = token.value();
        return std::nullopt;
    }
    else
    {
        return "Invalid token.";
    }
}

std::optional<std::string> ws_handle_logout(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto token = get_string(request_data, "token");
    if(!token)
    {
        return "Invalid JSON.";
    }

    std::string token_username = authentication_handler->authenticate(token.value());
    const std::string& connection_username = ctx.username;

    if(token_username == connection_username)
    {
        if(token == ctx.active_token)
        {
            //TODO clean subs?
            ctx.active_token = "";
            ctx.username = "";
        }
        authentication_handler->delete_token(token.value());
        return std::nullopt;
    }
    else
    {
        return "Invalid JSON.";
    }
}

std::optional<std::string> ws_handle_list_projects(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value array;
    array.SetArray();
    for(auto& [project, jobs] : projects->projects)
    {
        if(auto perm = rights->check_right(ctx.username, "project." + project.name);
                perm.has_value() && perm.value())
        {
            array.PushBack(
                    rapidjson::Value().CopyFrom(
                        to_json(project),
                        allocator),
                    allocator);
        }
    }
    response_data.AddMember("projects", array, allocator);
    return std::nullopt;
}

std::optional<std::string> ws_handle_list_jobs(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{    
    auto project_name = get_string(request_data, "project");
    if(!project_name)
    {
        return "Invalid JSON.";
    }

    auto project_it = projects->projects.find(project_name.value());
    auto perm = rights->check_right(ctx.username, "project." + project_name.value());
    auto permitted = perm.has_value() ? perm.value() : true;

    if(project_it != projects->projects.cend() && permitted)
    {
        rapidjson::Value array;
        array.SetArray();
        for(auto& [job, builds] : project_it->second)
        {
            array.PushBack(
                    rapidjson::Value().CopyFrom(
                        to_json(job),
                        allocator),
                    allocator);
        }
        response_data.AddMember("jobs", array, allocator);
        return std::nullopt;
    }
    else if(!permitted)
    {
        return "Action not permitted.";
    }
    else
    {
        return "Project doesn't exists.";
    }
}

std::optional<std::string> ws_handle_list_builds(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto project_name = get_string(request_data, "project");
    auto job_name = get_string(request_data, "job");
    if(!project_name || !job_name)
    {
        return "Invalid JSON.";
    }
   
    auto project_it = projects->projects.find(project_name.value());
    auto perm = rights->check_right(ctx.username, "project." + project_name.value());
    auto permitted = perm.has_value() ? perm.value() : true;

    if(project_it != projects->projects.cend() && permitted)
    {
        auto job_it = project_it->second.find(job_name.value());
        if(job_it != project_it->second.cend())
{
            rapidjson::Value array;
            array.SetArray();
            for(auto& build : job_it->second)
            {
                array.PushBack(
                        rapidjson::Value().CopyFrom(
                            to_json(build),
                            allocator),
                        allocator);
            }
            response_data.AddMember("builds", array, allocator);
            return std::nullopt;
        }
        else
        {
            return "Job doesn't exists.";
        }
    }
    else if(!permitted)
    {
        return "Action not permitted.";
    }
    else
    {
        return "Project doesn't exists.";
    }
}

std::optional<std::string> ws_handle_run_job(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        boost::asio::io_context& io_ctx,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto project_name = get_string(request_data, "project");
    auto job_name = get_string(request_data, "job");
    if(!project_name || !job_name)
    {
        return "Invalid JSON.";
    }
   
    auto project_it = projects->projects.find(project_name.value());
    auto perm = rights->check_right(ctx.username, "project." + project_name.value());
    auto permitted = perm.has_value() ? perm.value() : true;

    if(project_it != projects->projects.cend() && permitted)
    {
        auto job_it = project_it->second.find(job_name.value());
        if(job_it != project_it->second.cend())
        {
            run_job(io_ctx, project_name.value(), job_name.value());
            return std::nullopt;
            projects->defer_fetch();
            return std::nullopt;
        }
        else
        {
            return "Job doesn't exists.";
        }
    }
    else if(!permitted)
    {
        return "Action not permitted.";
    }
    else
    {
        return "Project doesn't exists.";
    }
}
