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

std::optional<bool> get_bool(
        const rapidjson::Value& value,
        const char* name)
{
    if(value.HasMember(name) && value[name].IsBool())
    {
        return value[name].GetBool();
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
        response_data.AddMember(
                "group",
                rapidjson::Value().SetString(
                    authentication_handler->is_admin(ctx.username)
                    ? "admin"
                    : "user",
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
        response_data.AddMember(
                "group",
                rapidjson::Value().SetString(
                    authentication_handler->is_admin(ctx.username)
                    ? "admin"
                    : "user",
                    allocator),
                allocator);
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
        if(auto perm = rights->check_project_right(ctx.username, project.name);
                (perm.has_value() && perm.value().read) || (!perm.has_value() && true))
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

std::optional<std::string> ws_handle_get_project_info(
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
    auto perm = rights->check_project_right(ctx.username, project_name.value());
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(project_it != projects->projects.cend() && permitted)
    {
        rapidjson::Value array;
        rapidjson::Value array_value;
        array.SetArray();
        for(auto& file : project_it->second.files)
        {
            array_value.SetObject();
            array_value.AddMember(
                    "name",
                    rapidjson::Value().SetString(
                        file.c_str(),
                        file.length(),
                        allocator),
                    allocator);
            array_value.AddMember(
                    "link",
                    rapidjson::Value().SetString(""),
                    allocator);
            array.PushBack(
                    array_value,
                    allocator);
        }
        response_data.AddMember("files", array, allocator);
        array.SetArray();
        for(auto& [job, builds] : project_it->second.jobs)
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

std::optional<std::string> ws_handle_get_job_info(
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
    auto perm = rights->check_project_right(ctx.username, project_name.value());
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(project_it != projects->projects.cend() && permitted)
    {
        auto job_it = project_it->second.jobs.find(job_name.value());
        if(job_it != project_it->second.jobs.cend())
        {
            rapidjson::Value array;
            rapidjson::Value array_value;
            array.SetArray();
            for(auto& file : job_it->second.files)
            {
                array_value.SetObject();
                array_value.AddMember(
                        "name",
                        rapidjson::Value().SetString(
                            file.c_str(),
                            file.length(),
                            allocator),
                        allocator);
                array_value.AddMember(
                        "link",
                        rapidjson::Value().SetString(""),
                        allocator);
                array.PushBack(
                        array_value,
                        allocator);
            }
            response_data.AddMember("files", array, allocator);
            array.SetArray();
            for(auto& param : job_it->second.params)
            {
                array_value.SetObject();
                array_value.AddMember(
                        "name",
                        rapidjson::Value().SetString(
                            param.name.c_str(),
                            param.name.length(),
                            allocator),
                        allocator);
                array_value.AddMember(
                        "default_value",
                        rapidjson::Value().SetString(
                            param.default_value.c_str(),
                            param.default_value.length(),
                            allocator),
                        allocator);
                array.PushBack(
                        array_value,
                        allocator);
            }
            response_data.AddMember("params", array, allocator);
            array.SetArray();
            for(auto& build : job_it->second.builds)
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
    auto perm = rights->check_project_right(ctx.username, project_name.value());
    auto permitted = perm.has_value() ? perm.value().execute : true;

    if(project_it != projects->projects.cend() && permitted)
    {
        auto job_it = project_it->second.jobs.find(job_name.value());
        if(job_it != project_it->second.jobs.cend())
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

std::optional<std::string> ws_handle_change_pass(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto old_pass = get_string(request_data, "oldPass");
    auto new_pass = get_string(request_data, "newPass");
    if(!old_pass || !new_pass)
    {
        return "Invalid JSON.";
    }
    bool ok = authentication_handler->change_pass(
            ctx.username,
            old_pass.value(),
            new_pass.value());
    if(!ok)
    {
        return "Invalid password";
    }
    return std::nullopt;
}

std::optional<std::string> ws_handle_list_users(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto perm = rights->check_user_right(ctx.username, "users.list");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        auto& users = authentication_handler->get_users();
        rapidjson::Value array;
        rapidjson::Value array_value;
        array.SetArray();
        for(auto& [username, user] : users)
        {
            array_value.SetObject();
            array_value.AddMember(
                    "name",
                    rapidjson::Value().SetString(
                        username.c_str(),
                        username.length(),
                        allocator),
                    allocator);
            array_value.AddMember(
                    "email",
                    rapidjson::Value().SetString(
                        user.email.c_str(),
                        user.email.length(),
                        allocator),
                    allocator);
            array_value.AddMember(
                    "group",
                    rapidjson::Value().SetString(
                        user.admin
                        ? "admin"
                        : "user",
                        allocator),
                    allocator);
            array_value.AddMember(
                    "disabled",
                    rapidjson::Value().SetBool(user.disabled),
                    allocator);
            if(user.api_access_key)
            {
                auto& key = user.api_access_key.value();
                array_value.AddMember(
                        "APIAccessSecretKey",
                        rapidjson::Value().SetString(
                            key.c_str(),
                            key.length(),
                            allocator),
                        allocator);
            }
            else
            {
                array_value.AddMember("APIAccessSecretKey", "", allocator);
            }
            array.PushBack(array_value, allocator);
        }
        response_data.AddMember("users", array, allocator);
        return std::nullopt;
    }
    return "Action not permitted";
}

std::optional<std::string> ws_handle_get_user_permissions(
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto name = get_string(request_data, "name");
    if(!name)
    {
        return "Invalid JSON.";
    }

    auto perm = rights->check_user_right(ctx.username, "users.permissions");
    auto permitted = perm.has_value() ? perm.value() : false;
    
    if(permitted)
    {
        auto& permissions = rights->get_permissions(name.value());
        rapidjson::Value array;
        rapidjson::Value array_value;
        array.SetArray();
        for(auto [project, rights] : permissions)
        {
            array_value.SetObject();
            array_value.AddMember(
                    "name",
                    rapidjson::Value().SetString(
                        project.c_str(),
                        project.length(),
                        allocator),
                    allocator);
            array_value.AddMember(
                    "read",
                    rapidjson::Value().SetBool(rights.read),
                    allocator);
            array_value.AddMember(
                    "write",
                    rapidjson::Value().SetBool(rights.write),
                    allocator);
            array_value.AddMember(
                    "execute",
                    rapidjson::Value().SetBool(rights.execute),
                    allocator);
            array.PushBack(array_value, allocator);
        }
        response_data.AddMember("permissions", array, allocator);
        return std::nullopt;
    }
    return "Action not permitted";
}

std::optional<std::string> ws_handle_set_user_permissions(
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto name = get_string(request_data, "name");
    if(!name || !request_data.HasMember("permissions") || !request_data["permissions"].IsArray())
    {
        return "Invalid JSON.";
    }

    auto perm = rights->check_user_right(ctx.username, "users.permissions");
    auto permitted = perm.has_value() ? perm.value() : false;
    
    if(permitted)
    {
        for(auto& array_value : request_data["permissions"].GetArray())
        {
            auto project_name = get_string(array_value, "name");
            auto read = get_bool(array_value, "read");
            auto write = get_bool(array_value, "write");
            auto execute = get_bool(array_value, "execute");
            if(!project_name || !read || !write || !execute)
            {
                return "Invalid JSON.";
            }
            rights->set_user_project_permissions(
                    name.value(),
                    project_name.value(),
                    {read.value(), write.value(), execute.value()});
        }
        return std::nullopt;
    }
    return "Action not permitted";
}

std::optional<std::string> ws_handle_change_group(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto name = get_string(request_data, "name");
    auto group = get_string(request_data, "admin");
    if(!name || !group)
    {
        return "Invalid JSON.";
    }

    if(authentication_handler->has_user(name.value()))
    {
        return "Invalid username.";
    }

    bool state;
    if(group.value() == "admin")
    {
        state = true;
    }
    else if(group.value() == "user")
    {
        state = false;
    }
    else
    {
        return "Invalid group name.";
    }

    auto perm = rights->check_user_right(ctx.username, "users.make_admin");
    auto permitted = perm.has_value() ? perm.value() : false;
    if(permitted)
    {
        authentication_handler->make_admin(name.value(), state);
        return std::nullopt;
    }
    return "Action not permitted.";
}

std::optional<std::string> ws_handle_disable_user(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto name = get_string(request_data, "name");
    auto state = get_bool(request_data, "state");
    if(!name || !state)
    {
        return "Invalid JSON.";
    }
    
    if(authentication_handler->has_user(name.value()))
    {
        return "Invalid username.";
    }

    auto perm = rights->check_user_right(ctx.username, "users.disable");
    auto permitted = perm.has_value() ? perm.value() : false;
    if(permitted)
    {
        authentication_handler->set_disabled(name.value(), state.value());
        return std::nullopt;
    }
    return "Action not permitted.";
}

std::optional<std::string> ws_handle_generate_api_key(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto name = get_string(request_data, "name");
    if(!name)
    {
        return "Invalid JSON.";
    }

    if(ctx.username == name.value() 
        || authentication_handler->is_admin(ctx.username))
    {
        auto api_key = authentication_handler->generate_api_key(name.value());
        if(!api_key)
        {
            return "Can't generate APIAccessSecretKey.";
        }
        response_data.AddMember(
                "APIAccessSecretKey",
                rapidjson::Value().SetString(
                    api_key.value().c_str(),
                    api_key.value().length(),
                    allocator),
                allocator);
        return std::nullopt;
    }
    return "Action not permitted.";
}

std::optional<std::string> ws_handle_rename_job(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto project_name = get_string(request_data, "project");
    auto job_name = get_string(request_data, "job");
    auto new_job_name = get_string(request_data, "name");
    if(!project_name || !job_name || !new_job_name)
    {
        return "Invalid JSON.";
    }

    if(new_job_name.value().empty())
    {
        return "Empty name field.";
    }
   
    auto project_it = projects->projects.find(project_name.value());
    auto perm = rights->check_project_right(ctx.username, project_name.value());
    auto permitted = perm.has_value() ? perm.value().write : true;

    if(project_it != projects->projects.cend() && permitted)
    {
        if(auto it = project_it->second.jobs.find(new_job_name.value());
                it != project_it->second.jobs.cend())
        {
            return "Project with this name already exists.";
        }

        auto job_it = project_it->second.jobs.find(job_name.value());
        if(job_it != project_it->second.jobs.cend())
        {
            std::error_code ec;
            rename_job(project_name.value(), job_name.value(), new_job_name.value(), ec);
            if(ec)
            {
                return "Error while renaming.";
            }
            projects->fetch();
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

std::optional<std::string> ws_handle_get_build_info(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto project_name = get_string(request_data, "project");
    auto job_name = get_string(request_data, "job");
    auto build_name = get_string(request_data, "build");
    if(!project_name || !job_name || !build_name)
    {
        return "Invalid JSON.";
    }

    auto project_it = projects->projects.find(project_name.value());
    auto perm = rights->check_project_right(ctx.username, project_name.value());
    auto permitted = perm.has_value() ? perm.value().write : true;
    if(project_it != projects->projects.cend() && permitted)
    {   
        auto job_it = project_it->second.jobs.find(job_name.value());
        if(job_it != project_it->second.jobs.cend())
        {
            auto build_it = job_it->second.builds.find(build_name.value());
            if(build_it != job_it->second.builds.cend())
            {
                rapidjson::Value value;
                value.SetInt(build_it->status);
                response_data.AddMember("status", value, allocator);
                value.SetString(
                        build_it->date.c_str(),
                        build_it->date.length(),
                        allocator);
                response_data.AddMember("date", value, allocator);
                value.SetArray();
                rapidjson::Value array_value;
                for(auto& artifact : build_it->artifacts)
                {
                    array_value.SetObject();
                    array_value.AddMember(
                            "name",
                            rapidjson::Value().SetString(
                                artifact.c_str(),
                                artifact.length(),
                                allocator),
                            allocator);
                    array_value.AddMember(
                            "link",
                            rapidjson::Value().SetString(""),
                            allocator);
                    value.PushBack(
                            array_value,
                            allocator);
                }
                response_data.AddMember("artufacts", value, allocator);
                return std::nullopt;
            }
            else
            {
                return "Build doesn't exists.";
            }
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
