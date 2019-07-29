#include "event_handlers.h"

#include <rapidjson/writer.h>

#include "cron_utils.h"
#include "event_list.h"
#include "bound_task_chain.h"

#include "websocket/dto/auth_login_pass_response.h"
#include "websocket/dto/auth_token_response.h"
#include "websocket/dto/logout_response.h"
#include "websocket/dto/change_pass_response.h"
#include "websocket/dto/get_user_list_response.h"
#include "websocket/dto/get_user_permissions_response.h"
#include "websocket/dto/set_user_permissions_response.h"
#include "websocket/dto/change_group_response.h"
#include "websocket/dto/disable_user_response.h"
#include "websocket/dto/generate_api_key_response.h"
#include "websocket/dto/get_api_key_response.h"
#include "websocket/dto/remove_api_key_response.h"
#include "websocket/dto/get_project_list_response.h"
#include "websocket/dto/get_project_info_response.h"
#include "websocket/dto/get_job_info_response.h"
#include "websocket/dto/run_job_response.h"
#include "websocket/dto/get_build_info_response.h"
#include "websocket/dto/refresh_fs_entry_response.h"
#include "websocket/dto/remove_fs_entry_response.h"
#include "websocket/dto/move_fs_entry_response.h"
#include "websocket/dto/new_directory_response.h"
#include "websocket/dto/list_directory_response.h"
#include "websocket/dto/add_cis_cron_response.h"
#include "websocket/dto/remove_cis_cron_response.h"
#include "websocket/dto/list_cis_cron_response.h"

namespace websocket
{

namespace handlers
{

void authenticate(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_login_pass_request& req,
        transaction tr)
{
    auto token = authentication_handler.authenticate(
            req.username,
            req.pass);

    if(token)
    {
        ctx.username = req.username;
        ctx.active_token = token.value();

        auto group = authentication_handler.get_group(ctx.username);

        if(group)
        {
            dto::auth_login_pass_response res;
            res.token = token.value();
            res.group = group.value();

            return tr.send(res);
        }

        return tr.send_error("Internal error.");
    }

    return tr.send_error("Wrong username or password.");
}

void token(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_token_request& req,
        transaction tr)
{
    auto username = authentication_handler.authenticate(req.token);

    if(username)
    {
        ctx.username = username.value();
        ctx.active_token = req.token;

        auto group = authentication_handler.get_group(ctx.username);

        if(group)
        {
            dto::auth_token_response res;
            res.group = group.value();

            return tr.send(res);
        }

        return tr.send_error("Internal error.");
    }

    return tr.send_error("Invalid token.");
}

void logout(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::logout_request& req,
        transaction tr)
{
    auto username = authentication_handler.authenticate(req.token);

    if(username && username.value() == ctx.username)
    {
        if(req.token == ctx.active_token)
        {
            //TODO clean subs?
            ctx.active_token = "";
            ctx.username = "";
        }

        authentication_handler.delete_token(req.token);

        dto::logout_response res;

        return tr.send(res);
    }

    return tr.send_error("Invalid token.");
}

void list_projects(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::get_project_list_request& req,
        transaction tr)
{
    dto::get_project_list_response res;

    for(auto& [project_name, project] : cis_manager.get_projects())
    {
        if(auto perm = rights.check_project_right(ctx.username, project_name);
                (perm.has_value() && perm.value().read) || !perm.has_value())
        {
            res.projects.push_back({project_name});
        }
    }

    return tr.send(res);
}

void get_project_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::get_project_info_request& req,
        transaction tr)
{
    auto* project = cis_manager.get_project_info(req.project);

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(project != nullptr && permitted)
    {
        dto::get_project_info_response res;

        for(auto& file : project->get_files())
        {
            bool is_directory = file.dir_entry().is_directory();
            auto path = ("/" / file.relative_path()).string();
            auto link = ("/download" / file.relative_path()).string();

            res.fs_entries.push_back(dto::fs_entry{
                    file.filename(),
                    false,
                    is_directory,
                    path,
                    link});
        }

        for(auto& [job_name, job] : project->get_jobs())
        {
            res.jobs.push_back({job_name});
        }

        return tr.send(res);
    }

    if(!permitted)
    {
        return tr.send_error("Action not permitted.");
    }

    return tr.send_error("Project doesn't exists.");
}

void get_job_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::get_job_info_request& req,
        transaction tr)
{
    auto* job = cis_manager.get_job_info(req.project, req.job);

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(job != nullptr && permitted)
    {
        dto::get_job_info_response res;

        for(auto& file : job->get_files())
        {
            bool is_directory = file.dir_entry().is_directory();
            auto path = ("/" / file.relative_path()).string();
            auto link = ("/download" / file.relative_path()).string();

            res.fs_entries.push_back(dto::fs_entry{
                    file.filename(),
                    false,
                    is_directory,
                    path,
                    link});
        }

        for(auto& param : job->get_params())
        {
            res.params.push_back({param.name, param.default_value});
        }

        for(auto& [build_name, build] : job->get_builds())
        {
            auto& info = build.get_info();

            res.builds.push_back({
                    build_name,
                    info.status ? info.status.value() : -1,
                    info.date ? info.date.value() : ""});
        }

        return tr.send(res);
    }

    if(!permitted)
    {
        return tr.send_error("Action not permitted.");
    }

    return tr.send_error("Job doesn't exists.");
}

void run_job(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        boost::asio::io_context& io_ctx,
        request_context& ctx,
        const dto::run_job_request& req,
        transaction tr)
{
    std::map<std::string, std::string> params;

    for(auto& param : req.params)
    {
        params.insert({param.name, param.value});
    }

    auto* job = cis_manager.get_job_info(req.project, req.job);

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().execute : true;

    if(job != nullptr && permitted)
    {
        auto& job_params = job->get_params();
        std::vector<std::string> param_values;
        param_values.reserve(job_params.size());

        for(auto& param : job_params)
        {
            if(!params.count(param.name))
            {
                if(param.default_value.empty())
                {
                    return tr.send_error("Invalid params.");
                }
                param_values.push_back(param.default_value);
            }
            else
            {
                param_values.push_back(params[param.name]);
            }
        }

        cis_manager.run_job(
                req.project,
                req.job,
                param_values);

        dto::run_job_response res;

        return tr.send(res);
    }

    if(!permitted)
    {
        return tr.send_error("Action not permitted.");
    }

    return tr.send_error("Job doesn't exists.");
}

void change_pass(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::change_pass_request& req,
        transaction tr)
{
    bool ok = authentication_handler.change_pass(
            ctx.username,
            req.old_password,
            req.new_password);

    if(!ok)
    {
        return tr.send_error("Invalid password");
    }

    dto::change_pass_response res;

    return tr.send(res);
}

void list_users(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::get_user_list_request& req,
        transaction tr)
{
    auto perm = rights.check_user_permission(ctx.username, "users.list");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        const auto users = authentication_handler.get_user_infos();

        dto::get_user_list_response res;

        for(auto& user : users)
        {
            res.users.push_back({
                    user.name,
                    user.email,
                    user.group,
                    false,
                    user.api_access_key ? user.api_access_key.value() : ""});
        }

        return tr.send(res);
    }

    return tr.send_error("Action not permitted");
}

void get_user_permissions(
        rights_manager& rights,
        request_context& ctx,
        const dto::get_user_permissions_request& req,
        transaction tr)
{
    auto perm = rights.check_user_permission(ctx.username, "users.permissions");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        const auto permissions = rights.get_permissions(req.username);

        dto::get_user_permissions_response res;

        for(auto [project_name, project_rights] : permissions)
        {
            res.permissions.push_back({
                        project_name,
                        project_rights.read,
                        project_rights.write,
                        project_rights.execute});
        }

        return tr.send(res);
    }

    return tr.send_error("Action not permitted");
}

void set_user_permissions(
        rights_manager& rights,
        request_context& ctx,
        const dto::set_user_permissions_request& req,
        transaction tr)
{
    auto perm = rights.check_user_permission(ctx.username, "users.permissions");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        for(auto& perm : req.permissions)
        {
            rights.set_user_project_permissions(
                    req.username,
                    perm.project,
                    {-1, -1, -1, perm.read, perm.write, perm.execute});
        }

        dto::set_user_permissions_response res;

        return tr.send(res);
    }

    return tr.send_error("Action not permitted");
}

void change_group(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::change_group_request& req,
        transaction tr)
{

    if(!authentication_handler.has_user(req.username))
    {
        return tr.send_error("Invalid username.");
    }

    auto perm = rights.check_user_permission(ctx.username, "users.change_group");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        authentication_handler.change_group(req.username, req.group);

        dto::change_group_response res;

        return tr.send(res);
    }

    return tr.send_error("Action not permitted.");
}

void disable_user(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::disable_user_request& req,
        transaction tr)
{

    if(!authentication_handler.has_user(req.username))
    {
        return tr.send_error("Invalid username.");
    }

    auto perm = rights.check_user_permission(ctx.username, "users.change_group");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        authentication_handler.change_group(
                req.username,
                req.state ? "disabled" : "user");

        dto::disable_user_response res;

        return tr.send(res);
    }

    return tr.send_error("Action not permitted.");
}

void generate_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::generate_api_key_request& req,
        transaction tr)
{
    if(ctx.username == req.username
        || (!ctx.username.empty()
        && authentication_handler.get_group(ctx.username).value() == "admin"))
    {
        auto api_key = authentication_handler.generate_api_key(req.username);

        if(!api_key)
        {
            return tr.send_error("Can't generate APIAccessSecretKey.");
        }

        dto::generate_api_key_response res;
        res.api_key = api_key.value();

        return tr.send(res);
    }

    return tr.send_error("Action not permitted.");
}

void get_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::get_api_key_request& req,
        transaction tr)
{

    if(ctx.username == req.username
        || (!ctx.username.empty()
        && authentication_handler.get_group(ctx.username).value() == "admin"))
    {
        auto api_key = authentication_handler.get_api_key(req.username);

        if(!api_key)
        {
            return tr.send_error("Can't retrieve APIAccessSecretKey.");
        }

        dto::get_api_key_response res;
        res.api_key = api_key.value();

        return tr.send(res);
    }

    return tr.send_error("Action not permitted.");
}

void remove_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::remove_api_key_request& req,
        transaction tr)
{
    if(ctx.username == req.username
        || (!ctx.username.empty()
        && authentication_handler.get_group(ctx.username).value() == "admin"))
    {
        auto result = authentication_handler.remove_api_key(req.username);

        if(!result)
        {
            return tr.send_error("Can't remove APIAccessSecretKey.");
        }

        dto::remove_api_key_response res;

        return tr.send(res);
    }

    return tr.send_error("Action not permitted.");
}

void get_build_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::get_build_info_request& req,
        transaction tr)
{
    auto* build = cis_manager.get_build_info(
            req.project,
            req.job,
            req.build);

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().write : true;

    if(build != nullptr && permitted)
    {
        auto& info = build->get_info();
        dto::get_build_info_response res;
        res.status = info.status ? info.status.value() : -1;
        res.date = info.date ? info.date.value() : "";

        for(auto& file : build->get_files())
        {
            bool is_directory = file.dir_entry().is_directory();
            auto path = ("/" / file.relative_path()).string();
            auto link = ("/download" / file.relative_path()).string();

            res.fs_entries.push_back(dto::fs_entry{
                    file.filename(),
                    false,
                    is_directory,
                    path,
                    link});
        }

        return tr.send(res);
    }

    if(!permitted)
    {
        return tr.send_error("Action not permitted.");
    }

    return tr.send_error("Build doesn't exists.");
}

bool validate_path(const std::filesystem::path& path)
{
    return path.root_path() == "/";
}

std::optional<database::project_user_right> get_path_rights(
        request_context& ctx,
        rights_manager& rights,
        const std::filesystem::path& path)
{
    if(path.begin() != path.end())
    {
        auto path_it = path.begin();
        ++path_it;
        if(path_it != path.end())
        {
            return rights.check_project_right(
                    ctx.username,
                    *path_it);
        }
    }

    return std::nullopt;
}

void refresh_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::refresh_fs_entry_request& req,
        transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        return tr.send_error("Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, path);
            path_rights && !path_rights.value().read)
    {
        return tr.send_error("Action not permitted.");
    }

    auto refresh_result = cis_manager.refresh(path);

    if(!refresh_result)
    {
        return tr.send_error("Path does not exists.");
    }

    dto::refresh_fs_entry_response res;

    return tr.send(res);
}

void remove_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::remove_fs_entry_request& req,
        transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        return tr.send_error("Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, path);
            path_rights && !path_rights.value().write)
    {
        return tr.send_error("Action not permitted.");
    }

    auto remove_result = cis_manager.remove(path);

    if(!remove_result)
    {
        return tr.send_error("Path does not exists.");
    }

    dto::remove_fs_entry_response res;

    return tr.send(res);
}

void move_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::move_fs_entry_request& req,
        transaction tr)
{

    std::filesystem::path old_path(req.old_path);
    std::filesystem::path new_path(req.new_path);

    if(!validate_path(old_path) || !validate_path(new_path))
    {
        return tr.send_error("Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, old_path);
            path_rights && !path_rights.value().write)
    {
        return tr.send_error("Action not permitted.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, new_path);
            path_rights && !path_rights.value().write)
    {
        return tr.send_error("Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    std::error_code ec;
    fs.move_entry(old_path, new_path, ec);

    if(ec)
    {
        return tr.send_error("Error on move.");
    }

    dto::move_fs_entry_response res;

    return tr.send(res);
}

void new_directory(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::new_directory_request& req,
        transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        return tr.send_error("Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, path);
            path_rights && !path_rights.value().write)
    {
        return tr.send_error("Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    std::error_code ec;
    fs.create_directory(path, ec);

    if(ec)
    {
        return tr.send_error("Error while creating directory.");
    }

    dto::new_directory_response res;

    return tr.send(res);
}

void list_directory(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::list_directory_request& req,
        transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        return tr.send_error("Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, path);
            path_rights && !path_rights.value().read)
    {
        return tr.send_error("Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    dto::list_directory_response res;

    for(auto& file : fs)
    {
        bool is_directory = file.dir_entry().is_directory();
        auto path = ("/" / file.relative_path()).string();
        auto link = ("/download" / file.relative_path()).string();

        res.entries.push_back(dto::fs_entry{
                file.filename(),
                false,
                is_directory,
                path,
                link});
    }

    return tr.send(res);
}

void add_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::add_cis_cron_request& req,
        transaction tr)
{

    if(!cron::validate_expr(req.cron_expr))
    {
        return tr.send_error("Invalid cron expression.");
    }

    auto* job = cis_manager.get_job_info(req.project, req.job);
    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted =
        perm.has_value() ? (perm.value().execute && perm.value().write) : true;

    if(job != nullptr && permitted)
    {
        cis_manager.add_cron(
                req.project,
                req.job,
                req.cron_expr);

        dto::add_cis_cron_response res;

        return tr.send(res);
    }

    if(!permitted)
    {
        return tr.send_error("Action not permitted.");
    }

    return tr.send_error("Job doesn't exists.");
}

void remove_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::remove_cis_cron_request& req,
        transaction tr)
{

    if(!cron::validate_expr(req.cron_expr))
    {
        return tr.send_error("Invalid cron expression.");
    }

    auto* job = cis_manager.get_job_info(req.project, req.job);
    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted =
        perm.has_value() ? (perm.value().execute && perm.value().write) : true;

    if(job != nullptr && permitted)
    {
        cis_manager.remove_cron(
                req.project,
                req.job,
                req.cron_expr);

        dto::remove_cis_cron_response res;

        return tr.send(res);
    }

    if(!permitted)
    {
        return tr.send_error("Action not permitted.");
    }

    return tr.send_error("Job doesn't exists.");
}

void list_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::list_cis_cron_request& req,
        transaction tr)
{
    if(auto executor = tr.get_executor(); executor)
    {
        make_async_chain(executor.value())
            .then(cis_manager.list_cron(req.mask))
            .then([tr](const std::vector<cis::cron_entry>& entries)
                    {
                        dto::list_cis_cron_response res;
                        for(auto& entry : entries)
                        {
                            res.entries.push_back({
                                    entry.project,
                                    entry.job,
                                    entry.cron_expr});
                        }
                        tr.send(res);
                    })
            .run();
    }
}

} // namespace handlers

} // namespace websocket
