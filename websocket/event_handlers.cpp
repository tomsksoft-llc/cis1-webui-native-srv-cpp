#include "event_handlers.h"

#include <chrono>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "event_list.h"

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

namespace websocket
{

namespace handlers
{

std::optional<std::string> authenticate(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto login = get_string(request_data, "username");
    auto pass = get_string(request_data, "pass");
    
    if(!login || !pass)
    {
        return "Invalid JSON.";
    }

    auto token = authentication_handler->authenticate(
            login.value(),
            pass.value());
    
    if(token)
    {
        ctx.username = login.value();
        auto group = authentication_handler->get_group(ctx.username).value();
        ctx.active_token = token.value();
        response_data.AddMember(
                "token",
                rapidjson::Value().SetString(
                    token.value().c_str(),
                    token.value().length(),
                    allocator),
                allocator);
        response_data.AddMember(
                "group",
                rapidjson::Value().SetString(
                    group.c_str(),
                    group.length(),
                    allocator),
                allocator);
        return std::nullopt;
    }
    else
    {
        return "Wrong username or password.";
    }
}

std::optional<std::string> token(
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

    auto username = authentication_handler->authenticate(token.value());
    
    if(username)
    {
        ctx.username = username.value();
        ctx.active_token = token.value();
        auto group = authentication_handler->get_group(ctx.username).value();
        response_data.AddMember(
                "group",
                rapidjson::Value().SetString(
                    group.c_str(),
                    group.length(),
                    allocator),
                allocator);
        return std::nullopt;
    }
    else
    {
        return "Invalid token.";
    }
}

std::optional<std::string> logout(
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

    auto username = authentication_handler->authenticate(token.value());

    if(username && username.value() == ctx.username)
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

std::optional<std::string> list_projects(
        const std::shared_ptr<cis::project_list>& projects,
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

std::optional<std::string> get_project_info(
        const std::shared_ptr<cis::project_list>& projects,
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

std::optional<std::string> get_job_info(
        const std::shared_ptr<cis::project_list>& projects,
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

std::optional<std::string> run_job(
        const std::shared_ptr<cis::project_list>& projects,
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
            cis::run_job(io_ctx, project_name.value(), job_name.value());
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

std::optional<std::string> change_pass(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto old_pass = get_string(request_data, "oldPassword");
    auto new_pass = get_string(request_data, "newPassword");
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

std::optional<std::string> list_users(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto perm = rights->check_user_permission(ctx.username, "users.list");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        const auto users = authentication_handler->get_users();
        rapidjson::Value array;
        rapidjson::Value array_value;
        array.SetArray();
        for(auto& user : users)
        {
            array_value.SetObject();
            array_value.AddMember(
                    "username",
                    rapidjson::Value().SetString(
                        user.name.c_str(),
                        user.name.length(),
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
                        user.group_id == 2 //FIXME
                        ? "admin"
                        : "user",
                        allocator),
                    allocator);
            array_value.AddMember(
                    "disabled",
                    rapidjson::Value().SetBool(false/*user.disabled*/),
                    allocator);
            if(false)//user.api_access_key)
            {
                /*
                auto& key = user.api_access_key.value();
                array_value.AddMember(
                        "APIAccessSecretKey",
                        rapidjson::Value().SetString(
                            key.c_str(),
                            key.length(),
                            allocator),
                        allocator);*/
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

std::optional<std::string> get_user_permissions(
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

    auto perm = rights->check_user_permission(ctx.username, "users.permissions");
    auto permitted = perm.has_value() ? perm.value() : false;
    
    if(permitted)
    {
        const auto permissions = rights->get_permissions(name.value());
        rapidjson::Value array;
        rapidjson::Value array_value;
        array.SetArray();
        for(auto [project_name, project_rights] : permissions)
        {
            array_value.SetObject();
            array_value.AddMember(
                    "username",
                    rapidjson::Value().SetString(
                        project_name.c_str(),
                        project_name.length(),
                        allocator),
                    allocator);
            array_value.AddMember(
                    "read",
                    rapidjson::Value().SetBool(project_rights.read),
                    allocator);
            array_value.AddMember(
                    "write",
                    rapidjson::Value().SetBool(project_rights.write),
                    allocator);
            array_value.AddMember(
                    "execute",
                    rapidjson::Value().SetBool(project_rights.execute),
                    allocator);
            array.PushBack(array_value, allocator);
        }
        response_data.AddMember("permissions", array, allocator);

        return std::nullopt;
    }

    return "Action not permitted";
}

std::optional<std::string> set_user_permissions(
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto name = get_string(request_data, "username");

    if(!name || !request_data.HasMember("permissions") || !request_data["permissions"].IsArray())
    {
        return "Invalid JSON.";
    }

    auto perm = rights->check_user_permission(ctx.username, "users.permissions");
    auto permitted = perm.has_value() ? perm.value() : false;
    
    if(permitted)
    {
        for(auto& array_value : request_data["permissions"].GetArray())
        {
            auto project_name = get_string(array_value, "project");
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
                    {-1, -1, -1, read.value(), write.value(), execute.value()});
        }

        return std::nullopt;
    }

    return "Action not permitted";
}

std::optional<std::string> change_group(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto name = get_string(request_data, "username");
    auto group = get_string(request_data, "group");

    if(!name || !group)
    {
        return "Invalid JSON.";
    }

    if(authentication_handler->has_user(name.value()))
    {
        return "Invalid username.";
    }

    auto perm = rights->check_user_permission(ctx.username, "users.change_group");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        authentication_handler->change_group(name.value(), group.value());

        return std::nullopt;
    }

    return "Action not permitted.";
}

std::optional<std::string> disable_user(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto name = get_string(request_data, "username");
    auto state = get_bool(request_data, "state");

    if(!name || !state)
    {
        return "Invalid JSON.";
    }
    
    if(authentication_handler->has_user(name.value()))
    {
        return "Invalid username.";
    }

    auto perm = rights->check_user_permission(ctx.username, "users.change_group");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        authentication_handler->change_group(
                name.value(),
                state.value() ? "disabled" : "user");
        return std::nullopt;
    }

    return "Action not permitted.";
}

std::optional<std::string> generate_api_key(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto name = get_string(request_data, "username");

    if(!name)
    {
        return "Invalid JSON.";
    }

    if(ctx.username == name.value() 
        || authentication_handler->get_group(ctx.username).value() == "admin")
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

std::optional<std::string> rename_job(
        const std::shared_ptr<cis::project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto project_name = get_string(request_data, "project");
    auto job_name = get_string(request_data, "oldName");
    auto new_job_name = get_string(request_data, "newName");

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
            cis::rename_job(project_name.value(), job_name.value(), new_job_name.value(), ec);
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

std::optional<std::string> get_build_info(
        const std::shared_ptr<cis::project_list>& projects,
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

} // namespace handlers

} // namespace websocket
