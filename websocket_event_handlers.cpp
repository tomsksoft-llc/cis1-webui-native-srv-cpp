#include "websocket_event_handlers.h"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "websocket_event_list.h"

using namespace std::string_literals;

std::optional<std::string> get_string(
        const rapidjson::Document& value,
        const char* name)
{
    if(value.HasMember("login") && value["login"].IsString())
    {
        return value["login"].GetString();
    }
    return std::nullopt;
}

void make_response(
        websocket_queue& queue,
        ws_response_id id,
        std::string error_message,
        std::function<void(rapidjson::Document&, rapidjson::Value&)> data_builder = {})
{
    rapidjson::Document document;
    rapidjson::Value value;
    document.SetObject();
    value.SetInt(static_cast<int>(id));
    document.AddMember("eventId", value, document.GetAllocator());
    value.SetObject();
    if(data_builder)
    {
        data_builder(document, value);
    }
    value.AddMember(
            "errorMessage",
            rapidjson::Value().SetString(
                error_message.c_str(),
                error_message.length(),
                document.GetAllocator()),
            document.GetAllocator());
    document.AddMember("data", value, document.GetAllocator());
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    document.Accept(writer);
    queue.send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
};

void ws_handle_authenticate(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{
    auto login = get_string(data, "login");
    auto pass = get_string(data, "pass");
    
    if(!login || !pass)
    {
        make_response(
                queue,
                ws_response_id::auth_login_pass,
                "Invalid JSON.");
        return;
    }

    std::string token = authentication_handler->authenticate(login.value(), pass.value());
    
    if(!token.empty())
    {
        ctx.username = login.value();
        ctx.active_token = token;
        make_response(
                queue,
                ws_response_id::auth_login_pass,
                "",
                [&](rapidjson::Document& document, rapidjson::Value& value)
                {
                    value.AddMember(
                            "token",
                            rapidjson::Value().SetString(
                                token.c_str(),
                                token.length(),
                                document.GetAllocator()),
                            document.GetAllocator());
                });
    }
    else
    {
        make_response(
                queue,
                ws_response_id::auth_login_pass,
                "Wrong username or password.");
    }
}

void ws_handle_token(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{
    auto token = get_string(data, "token");
    if(!token)
    {
        make_response(
                queue,
                ws_response_id::auth_token,
                "Invalid JSON.");
        return;
    }

    std::string username = authentication_handler->authenticate(token.value());
    
    if(!username.empty())
    {
        ctx.username = username;
        ctx.active_token = token.value();
        make_response(
                queue,
                ws_response_id::auth_token,
                "");
    }
    else
    {
        make_response(
                queue,
                ws_response_id::auth_token,
                "Invalid token.");
    }
}

void ws_handle_logout(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{
    auto token = get_string(data, "token");
    if(!token)
    {
        make_response(
                queue,
                ws_response_id::auth_logout,
                "Invalid JSON.");
        return;
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
        make_response(
                queue,
                ws_response_id::auth_logout,
                "");
    }
    else
    {
        make_response(
                queue,
                ws_response_id::auth_logout,
                "Invalid token.");
    }
}

void ws_handle_list_projects(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{
    make_response(
            queue,
            ws_response_id::projects_list,
            "",
            [&](rapidjson::Document& document, rapidjson::Value& value)
            {
                rapidjson::Value array;
                array.SetArray();
                for(auto& [project, subprojects] : projects->projects)
                {
                    if(auto perm = rights->check_right(ctx.username, "project." + project.name);
                            perm.has_value() && perm.value())
                    {
                        array.PushBack(
                                rapidjson::Value().CopyFrom(
                                    to_json(project),
                                    document.GetAllocator()),
                                document.GetAllocator());
                    }
                }
                value.AddMember("projects", array, document.GetAllocator());
            });
}

void ws_handle_list_subprojects(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{    
    auto project_name = get_string(data, "project");
    if(!project_name)
    {
        make_response(
                queue,
                ws_response_id::subprojects_list,
                "Invalid JSON.");
        return;
    }

    auto project_it = projects->projects.find(project_name.value());
    auto perm = rights->check_right(ctx.username, "project." + project_name.value());
    auto permitted = perm.has_value() ? perm.value() : true;

    if(project_it != projects->projects.cend() && permitted)
    {
        make_response(
                queue,
                ws_response_id::subprojects_list,
                "",
                [&](rapidjson::Document& document, rapidjson::Value& value)
                {
                    rapidjson::Value array;
                    array.SetArray();
                    for(auto& [subproject, builds] : project_it->second)
                    {
                        array.PushBack(
                                rapidjson::Value().CopyFrom(
                                    to_json(subproject),
                                    document.GetAllocator()),
                                document.GetAllocator());
                    }
                    value.AddMember("subprojects", array, document.GetAllocator());
                });
    }
    else if(!permitted)
    {
        make_response(
                queue,
                ws_response_id::subprojects_list,
                "Action not permitted.");
    }
    else
    {
        make_response(
                queue,
                ws_response_id::subprojects_list,
                "Project doesn't exists.");
    }
}

void ws_handle_list_builds(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{
    auto project_name = get_string(data, "project");
    auto subproject_name = get_string(data, "subproject");
    if(!project_name || !subproject_name)
    {
        make_response(
                queue,
                ws_response_id::builds_list,
                "Invalid JSON.");
        return;
    }
   
    auto project_it = projects->projects.find(project_name.value());
    auto perm = rights->check_right(ctx.username, "project." + project_name.value());
    auto permitted = perm.has_value() ? perm.value() : true;

    if(project_it != projects->projects.cend() && permitted)
    {
        auto subproject_it = project_it->second.find(subproject_name.value());
        if(subproject_it != project_it->second.cend())
        {
            make_response(
                queue,
                ws_response_id::builds_list,
                "",
                [&](rapidjson::Document& document, rapidjson::Value& value)
                {
                    rapidjson::Value array;
                    array.SetArray();
                    for(auto& build : subproject_it->second)
                    {
                        array.PushBack(
                                rapidjson::Value().CopyFrom(
                                    to_json(build),
                                    document.GetAllocator()),
                                document.GetAllocator());
                    }
                    value.AddMember("builds", array, document.GetAllocator());
                });
        }
        else
        {
            make_response(
                queue,
                ws_response_id::builds_list,
                "Subproject doesn't exists.");
        }
    }
    else if(!permitted)
    {
        make_response(
                queue,
                ws_response_id::builds_list,
                "Action not permitted.");
    }
    else
    {
        make_response(
                queue,
                ws_response_id::builds_list,
                "Project doesn't exists.");
    }
}

void ws_handle_run_job(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        boost::asio::io_context& io_ctx,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{
    auto project_name = get_string(data, "project");
    auto subproject_name = get_string(data, "subproject");
    if(!project_name || !subproject_name)
    {
        make_response(
                queue,
                ws_response_id::run_job,
                "Invalid JSON.");
        return;
    }
   
    auto project_it = projects->projects.find(project_name.value());
    auto perm = rights->check_right(ctx.username, "project." + project_name.value());
    auto permitted = perm.has_value() ? perm.value() : true;

    if(project_it != projects->projects.cend() && permitted)
    {
        auto subproject_it = project_it->second.find(subproject_name.value());
        if(subproject_it != project_it->second.cend())
        {
            run_job(io_ctx, project_name.value(), subproject_name.value());
            make_response(
                queue,
                ws_response_id::run_job,
                "");
        }
        else
        {
            make_response(
                queue,
                ws_response_id::run_job,
                "Subproject doesn't exists.");
        }
    }
    else if(!permitted)
    {
        make_response(
                queue,
                ws_response_id::run_job,
                "Action not permitted.");
    }
    else
    {
        make_response(
                queue,
                ws_response_id::run_job,
                "Project doesn't exists.");
    }
}
