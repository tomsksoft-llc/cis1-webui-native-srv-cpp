#include "websocket_event_handlers.h"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "websocket_event_list.h"

using namespace std::string_literals;

void ws_handle_authenticate(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{
    auto login = data.HasMember("login") && data["login"].IsString() ?
        data["login"].GetString() : ""s;
    auto pass = data.HasMember("pass") && data["pass"].IsString() ?
        data["pass"].GetString() : ""s;
    std::string token = authentication_handler->authenticate(login, pass);

    rapidjson::Document document;
    rapidjson::Value value;
    document.SetObject();
    value.SetInt(2);
    document.AddMember("eventId", value, document.GetAllocator());
    rapidjson::Value data_value;
    data_value.SetObject();
    value.SetString(token.c_str(), token.length(), document.GetAllocator());
    data_value.AddMember("token", value, document.GetAllocator());
    if(!token.empty())
    {
        ctx.username = login;
        ctx.active_token = token;
        value.SetString("");
    }
    else
    {
        value.SetString("Wrong username or password");
    }
    data_value.AddMember("errorMessage", value, document.GetAllocator());
    document.AddMember("data", data_value, document.GetAllocator());
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    document.Accept(writer);
    queue.send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}

void ws_handle_token(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{
    auto token = data.HasMember("token") && data["token"].IsString() ?
        data["token"].GetString() : ""s;
    std::string username = authentication_handler->authenticate(token);

    rapidjson::Document document;
    rapidjson::Value value;
    document.SetObject();
    value.SetInt(4);
    document.AddMember("eventId", value, document.GetAllocator());
    rapidjson::Value data_value;
    data_value.SetObject();
    if(!username.empty())
    {
        ctx.username = username;
        ctx.active_token = token;
        value.SetString("");
    }
    else
    {
        value.SetString("Invalid token");
    }
    data_value.AddMember("errorMessage", value, document.GetAllocator());
    document.AddMember("data", data_value, document.GetAllocator());
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    document.Accept(writer);
    queue.send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}

void ws_handle_logout(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{
    auto token = data.HasMember("token") && data["token"].IsString() ?
        data["token"].GetString() : ""s;
    std::string token_username = authentication_handler->authenticate(token);
    const std::string& connection_username = ctx.username;

    rapidjson::Document document;
    rapidjson::Value value;
    document.SetObject();
    value.SetInt(6);
    document.AddMember("eventId", value, document.GetAllocator());
    rapidjson::Value data_value;
    data_value.SetObject();
    if(token_username == connection_username)
    {
        if(token == ctx.active_token)
        {
            //TODO clean subs?
            ctx.active_token = "";
            ctx.username = "";
        }
        authentication_handler->delete_token(token);
        value.SetString("");
    }
    else
    {
        value.SetString("Invalid token");
    }
    data_value.AddMember("errorMessage", value, document.GetAllocator());
    document.AddMember("data", data_value, document.GetAllocator());
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    document.Accept(writer);
    queue.send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}

void ws_handle_list_projects(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{
    rapidjson::Document document;
    rapidjson::Value value;
    document.SetObject();
    value.SetInt(static_cast<int>(ws_response_id::projects_list));
    document.AddMember("eventId", value, document.GetAllocator());
    rapidjson::Value data_value;
    data_value.SetObject();
    rapidjson::Value array_value;
    value.SetArray();
    for(auto& [project, subprojects] : projects->projects)
    {
        if(auto perm = rights->check_right(ctx.username, "project." + project.name);
                perm.has_value() && perm.value())
        {
            array_value.CopyFrom(to_json(project), document.GetAllocator());
            value.PushBack(array_value, document.GetAllocator());
        }
    }
    data_value.AddMember("projects", value, document.GetAllocator());
    value.SetString("");
    data_value.AddMember("errorMessage", value, document.GetAllocator());
    document.AddMember("data", data_value, document.GetAllocator());
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    document.Accept(writer);
    queue.send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}

void ws_handle_list_subprojects(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{    
    auto project_name = data.HasMember("project") && data["project"].IsString() ?
        data["project"].GetString() : ""s;

    rapidjson::Document document;
    rapidjson::Value value;
    document.SetObject();
    value.SetInt(static_cast<int>(ws_response_id::subprojects_list));
    document.AddMember("eventId", value, document.GetAllocator());
    rapidjson::Value data_value;
    data_value.SetObject();
    rapidjson::Value array_value;
    value.SetArray();
    auto project_it = projects->projects.find(project_name);
    if(project_it != projects->projects.cend())
    {
        if(auto perm = rights->check_right(ctx.username, "project." + project_name);
                    perm.has_value() && perm.value())
        {
            for(auto& [subproject, builds] : project_it->second)
            {

                    array_value.CopyFrom(to_json(subproject), document.GetAllocator());
                    value.PushBack(array_value, document.GetAllocator());
            }
            data_value.AddMember("subprojects", value, document.GetAllocator());
            value.SetString("");
        }
        else
        {
            value.SetString("Action not permitted.");
        }
    }
    else
    {
        value.SetString("Project doesen't exist.");
    }
    data_value.AddMember("errorMessage", value, document.GetAllocator());
    document.AddMember("data", data_value, document.GetAllocator());
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    document.Accept(writer);
    queue.send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}

void ws_handle_list_builds(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{
    auto project_name = data.HasMember("project") && data["project"].IsString() ?
        data["project"].GetString() : ""s;
    auto subproject_name = data.HasMember("subproject") && data["subproject"].IsString() ?
        data["subproject"].GetString() : ""s;
    
    rapidjson::Document document;
    rapidjson::Value value;
    document.SetObject();
    value.SetInt(static_cast<int>(ws_response_id::builds_list));
    document.AddMember("eventId", value, document.GetAllocator());
    rapidjson::Value data_value;
    data_value.SetObject();
    rapidjson::Value array_value;
    value.SetArray();
    auto project_it = projects->projects.find(project_name);
    if(project_it != projects->projects.cend())
    {
        auto subproject_it = project_it->second.find(subproject_name);
        if(subproject_it != project_it->second.cend())
        {
            if(auto perm = rights->check_right(ctx.username, "project." + project_name);
                        perm.has_value() && perm.value())
            {
                for(auto& build : subproject_it->second)
                {

                        array_value.CopyFrom(to_json(build), document.GetAllocator());
                        value.PushBack(array_value, document.GetAllocator());
                }
                data_value.AddMember("builds", value, document.GetAllocator());
                value.SetString("");
            }
            else
            {
                value.SetString("Action not permitted.");
            }
        }
        else
        {
            value.SetString("Subproject doesn't exist.");
        }
    }
    else
    {
        value.SetString("Project doesen't exist.");
    }
    data_value.AddMember("errorMessage", value, document.GetAllocator());
    document.AddMember("data", data_value, document.GetAllocator());
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    document.Accept(writer);
    queue.send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}

void ws_handle_run_job(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        boost::asio::io_context& io_ctx,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx)
{
    auto project_name = data.HasMember("project") && data["project"].IsString() ?
        data["project"].GetString() : ""s;
    auto subproject_name = data.HasMember("subproject") && data["subproject"].IsString() ?
        data["subproject"].GetString() : ""s;
    rapidjson::Document document;
    rapidjson::Value value;
    document.SetObject();
    value.SetInt(static_cast<int>(ws_response_id::run_job));
    document.AddMember("eventId", value, document.GetAllocator());
    rapidjson::Value data_value;
    data_value.SetObject();
    auto project_it = projects->projects.find(project_name);
    if(project_it != projects->projects.cend())
    {
        auto subproject_it = project_it->second.find(subproject_name);
        if(subproject_it != project_it->second.cend())
        {
            if(auto perm = rights->check_right(ctx.username, "project." + project_name);
                        perm.has_value() && perm.value())
            {
                run_job(io_ctx, project_name, subproject_name);
                value.SetString("");
            }
            else
            {
                value.SetString("Action not permitted.");
            }
        }
        else
        {
            value.SetString("Subproject doesn't exist.");
        }
    }
    else
    {
        value.SetString("Project doesen't exist.");
    }
    data_value.AddMember("errorMessage", value, document.GetAllocator());
    document.AddMember("data", data_value, document.GetAllocator());
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    document.Accept(writer);
    queue.send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}
