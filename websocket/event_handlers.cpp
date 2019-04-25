#include "event_handlers.h"

#include <rapidjson/writer.h>

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
        auth_manager& authentication_handler,
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

    auto token = authentication_handler.authenticate(
            login.value(),
            pass.value());

    if(token)
    {
        ctx.username = login.value();
        auto group = authentication_handler.get_group(ctx.username).value();
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

    return "Wrong username or password.";
}

std::optional<std::string> token(
        auth_manager& authentication_handler,
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

    auto username = authentication_handler.authenticate(token.value());

    if(username)
    {
        ctx.username = username.value();
        ctx.active_token = token.value();
        auto group = authentication_handler.get_group(ctx.username).value();
        response_data.AddMember(
                "group",
                rapidjson::Value().SetString(
                        group.c_str(),
                        group.length(),
                        allocator),
                allocator);
        return std::nullopt;
    }

    return "Invalid token.";
}

std::optional<std::string> logout(
        auth_manager& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& /*response_data*/,
        rapidjson::Document::AllocatorType& /*allocator*/)
{
    auto token = get_string(request_data, "token");
    if(!token)
    {
        return "Invalid JSON.";
    }

    auto username = authentication_handler.authenticate(token.value());

    if(username && username.value() == ctx.username)
    {
        if(token == ctx.active_token)
        {
            //TODO clean subs?
            ctx.active_token = "";
            ctx.username = "";
        }
        authentication_handler.delete_token(token.value());
        return std::nullopt;
    }

    return "Invalid JSON.";
}

std::optional<std::string> list_projects(
        cis::project_list& projects,
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& /*request_data*/,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value array;
    array.SetArray();
    for(auto& [project, jobs] : projects.get())
    {
        if(auto perm = rights.check_project_right(ctx.username, project.name);
                (perm.has_value() && perm.value().read) || !perm.has_value())
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
        cis::project_list& projects,
        rights_manager& rights,
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

    auto project_it = projects.get().find(project_name.value());
    auto perm = rights.check_project_right(ctx.username, project_name.value());
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(project_it != projects.get().cend() && permitted)
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

    if(!permitted)
    {
        return "Action not permitted.";
    }

    return "Project doesn't exists.";
}

std::optional<std::string> get_job_info(
        cis::project_list& projects,
        rights_manager& rights,
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

    auto project_it = projects.get().find(project_name.value());
    auto perm = rights.check_project_right(ctx.username, project_name.value());
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(project_it != projects.get().cend() && permitted)
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

    if(!permitted)
    {
        return "Action not permitted.";
    }

    return "Project doesn't exists.";
}

std::optional<std::string> run_job(
        cis::project_list& projects,
        rights_manager& rights,
        boost::asio::io_context& io_ctx,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& /*response_data*/,
        rapidjson::Document::AllocatorType& /*allocator*/)
{
    auto project_name = get_string(request_data, "project");
    auto job_name = get_string(request_data, "job");
    if(!project_name || !job_name
            || !(request_data.HasMember("params") && request_data["params"].IsArray()))
    {
        return "Invalid JSON.";
    }

    std::map<std::string, std::string> params;
    for(auto& param : request_data["params"].GetArray())
    {
        if(param.IsObject())
        {
            auto param_name = get_string(param, "name");
            auto param_value = get_string(param, "value");
            if(param_name && param_value)
            {
                params.insert_or_assign(
                        param_name.value(),
                        param_value.value());
            }
            else
            {
                return "Invalid JSON.";
            }
        }
        else
        {
            return "Invalid JSON.";
        }
    }

    auto project_it = projects.get().find(project_name.value());
    auto perm = rights.check_project_right(ctx.username, project_name.value());
    auto permitted = perm.has_value() ? perm.value().execute : true;

    if(project_it != projects.get().cend() && permitted)
    {
        auto job_it = project_it->second.jobs.find(job_name.value());
        if(job_it != project_it->second.jobs.cend())
        {
            auto& job_params = job_it->second.params;
            std::vector<std::string> param_values;
            param_values.reserve(job_params.size());
            for(auto& param : job_params)
            {
                if(!params.count(param.name))
                {
                    if(param.default_value.empty())
                    {
                        return "Invalid params.";
                    }
                    param_values.push_back(param.default_value);
                }
                else
                {
                    param_values.push_back(params[param.name]);
                }
            }
            cis::run_job(
                    io_ctx,
                    project_name.value(),
                    job_name.value(),
                    param_values);
            projects.defer_fetch();
            return std::nullopt;
        }
        else
        {
            return "Job doesn't exists.";
        }
    }

    if(!permitted)
    {
        return "Action not permitted.";
    }

    return "Project doesn't exists.";
}

std::optional<std::string> change_pass(
        auth_manager& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& /*response_data*/,
        rapidjson::Document::AllocatorType& /*allocator*/)
{
    auto old_pass = get_string(request_data, "oldPassword");
    auto new_pass = get_string(request_data, "newPassword");
    if(!old_pass || !new_pass)
    {
        return "Invalid JSON.";
    }
    bool ok = authentication_handler.change_pass(
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
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& /*request_data*/,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator)
{
    auto perm = rights.check_user_permission(ctx.username, "users.list");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        const auto users = authentication_handler.get_user_infos();
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
                            user.group.c_str(),
                            user.group.length(),
                            allocator),
                    allocator);
            array_value.AddMember(
                    "disabled",
                    rapidjson::Value().SetBool(
                            user.group == "disabled"),
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

std::optional<std::string> get_user_permissions(
        rights_manager& rights,
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

    auto perm = rights.check_user_permission(ctx.username, "users.permissions");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        const auto permissions = rights.get_permissions(name.value());
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
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& /*response_data*/,
        rapidjson::Document::AllocatorType& /*allocator*/)
{
    auto name = get_string(request_data, "username");

    if(!name || !request_data.HasMember("permissions") || !request_data["permissions"].IsArray())
    {
        return "Invalid JSON.";
    }

    auto perm = rights.check_user_permission(ctx.username, "users.permissions");
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
            rights.set_user_project_permissions(
                    name.value(),
                    project_name.value(),
                    {-1, -1, -1, read.value(), write.value(), execute.value()});
        }

        return std::nullopt;
    }

    return "Action not permitted";
}

std::optional<std::string> change_group(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& /*response_data*/,
        rapidjson::Document::AllocatorType& /*allocator*/)
{
    auto name = get_string(request_data, "username");
    auto group = get_string(request_data, "group");

    if(!name || !group)
    {
        return "Invalid JSON.";
    }

    if(!authentication_handler.has_user(name.value()))
    {
        return "Invalid username.";
    }

    auto perm = rights.check_user_permission(ctx.username, "users.change_group");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        authentication_handler.change_group(name.value(), group.value());

        return std::nullopt;
    }

    return "Action not permitted.";
}

std::optional<std::string> disable_user(
        auth_manager& authentication_handler,
        rights_manager& rights,
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

    if(!authentication_handler.has_user(name.value()))
    {
        return "Invalid username.";
    }

    auto perm = rights.check_user_permission(ctx.username, "users.change_group");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        authentication_handler.change_group(
                name.value(),
                state.value() ? "disabled" : "user");
        return std::nullopt;
    }

    return "Action not permitted.";
}

std::optional<std::string> generate_api_key(
        auth_manager& authentication_handler,
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
        || authentication_handler.get_group(ctx.username).value() == "admin")
    {
        auto api_key = authentication_handler.generate_api_key(name.value());
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
        cis::project_list& projects,
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& /*response_data*/,
        rapidjson::Document::AllocatorType& /*allocator*/)
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

    auto project_it = projects.get().find(project_name.value());
    auto perm = rights.check_project_right(ctx.username, project_name.value());
    auto permitted = perm.has_value() ? perm.value().write : true;

    if(project_it != projects.get().cend() && permitted)
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
            cis::rename_job(
                    project_name.value(),
                    job_name.value(),
                    new_job_name.value(),
                    ec);
            if(ec)
            {
                return "Error while renaming.";
            }
            projects.fetch();

            return std::nullopt;
        }

        return "Job doesn't exists.";
    }

    if(!permitted)
    {
        return "Action not permitted.";
    }

    return "Project doesn't exists.";
}

std::optional<std::string> get_build_info(
        cis::project_list& projects,
        rights_manager& rights,
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

    auto project_it = projects.get().find(project_name.value());
    auto perm = rights.check_project_right(ctx.username, project_name.value());
    auto permitted = perm.has_value() ? perm.value().write : true;
    if(project_it != projects.get().cend() && permitted)
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

            return "Build doesn't exists.";
        }

        return "Job doesn't exists.";
    }

    if(!permitted)
    {
        return "Action not permitted.";
    }

    return "Project doesn't exists.";
}

} // namespace handlers

} // namespace websocket
