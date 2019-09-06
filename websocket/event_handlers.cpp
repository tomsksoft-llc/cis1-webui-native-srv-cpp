#include "event_handlers.h"

#include <map>
#include <vector>

#include <rapidjson/writer.h>

#include "cron_utils.h"
#include "bound_task_chain.h"

#include "websocket/dto/auth_login_pass_success.h"
#include "websocket/dto/auth_logout_success.h"
#include "websocket/dto/auth_error_wrong_credentials.h"
#include "websocket/dto/user_auth_change_pass_success.h"
#include "websocket/dto/user_auth_error_pass_doesnt_match.h"
#include "websocket/dto/user_auth_error_user_not_found.h"
#include "websocket/dto/user_list_success.h"
#include "websocket/dto/user_permissions_get_success.h"
#include "websocket/dto/user_permissions_set_success.h"
#include "websocket/dto/user_auth_change_group_success.h"
#include "websocket/dto/user_auth_ban_success.h"
#include "websocket/dto/user_api_key_generate_success.h"
#include "websocket/dto/user_api_key_get_success.h"
#include "websocket/dto/user_api_key_remove_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/cis_project_list_get_success.h"
#include "websocket/dto/cis_project_info_success.h"
#include "websocket/dto/cis_project_error_doesnt_exist.h"
#include "websocket/dto/cis_job_info_success.h"
#include "websocket/dto/cis_job_run_success.h"
#include "websocket/dto/cis_job_error_doesnt_exist.h"
#include "websocket/dto/cis_job_error_invalid_params.h"
#include "websocket/dto/cis_build_info_success.h"
#include "websocket/dto/cis_build_error_doesnt_exist.h"
#include "websocket/dto/fs_entry_refresh_success.h"
#include "websocket/dto/fs_entry_remove_success.h"
#include "websocket/dto/fs_entry_move_success.h"
#include "websocket/dto/fs_entry_new_dir_success.h"
#include "websocket/dto/fs_entry_list_success.h"
#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/fs_entry_error_doesnt_exist.h"
#include "websocket/dto/fs_entry_error_cant_move.h"
#include "websocket/dto/fs_entry_error_cant_create_dir.h"
#include "websocket/dto/cis_cron_add_success.h"
#include "websocket/dto/cis_cron_remove_success.h"
#include "websocket/dto/cis_cron_list_success.h"

namespace websocket
{

namespace handlers
{

void authenticate(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_login_pass& req,
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
            dto::auth_login_pass_success res;
            res.token = token.value();
            res.group = group.value();

            return tr.send(res);
        }

        return tr.send_error("Internal error.");
    }

    dto::auth_error_wrong_credentials err;

    return tr.send_error(err, "Wrong credentials.");
}

void token(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_token& req,
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
            dto::auth_login_pass_success res;
            res.token = req.token;
            res.group = group.value();

            return tr.send(res);
        }

        return tr.send_error("Internal error.");
    }

    dto::auth_error_wrong_credentials err;

    return tr.send_error(err, "Invalid token.");
}

void logout(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_logout& req,
        transaction tr)
{
    auto username = authentication_handler.authenticate(req.token);

    if(username && username.value() == ctx.username)
    {
        if(req.token == ctx.active_token)
        {
            //TODO clean subs?
            ctx.active_token.clear();
            ctx.username.clear();
        }

        authentication_handler.delete_token(req.token);

        return tr.send(dto::auth_logout_success{});
    }

    dto::auth_error_wrong_credentials err;
    
    return tr.send_error(err, "Invalid token.");
}

void list_projects(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_project_list_get& req,
        transaction tr)
{
    dto::cis_project_list_get_success res;

    for(auto& file : cis_manager.fs())
    {
        bool is_directory = file.dir_entry().is_directory();
        auto path = ("/" / file.relative_path()).generic_string();
        auto link = ("/download" / file.relative_path()).generic_string();

        res.fs_entries.push_back(dto::fs_entry{
                file.filename(),
                false,
                is_directory,
                path,
                link});
    }

    for(auto& [project_name, project] : cis_manager.get_projects())
    {
        auto it = std::find_if(
                res.fs_entries.begin(),
                res.fs_entries.end(),
                [&project_name](auto& entry)
                {
                    return entry.name == project_name;
                });

        bool permitted = false;

        if(auto perm = rights.check_project_right(ctx.username, project_name);
                (perm.has_value() && perm.value().read) || !perm.has_value())
        {
            permitted = true;
        }

        it->metainfo = dto::fs_entry::project_info{true};
    }

    return tr.send(res);
}

void get_project_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_project_info& req,
        transaction tr)
{
    auto* project = cis_manager.get_project_info(req.project);

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(project != nullptr && permitted)
    {
        dto::cis_project_info_success res;

        for(auto& file : project->get_files())
        {
            bool is_directory = file.dir_entry().is_directory();
            auto path = ("/" / file.relative_path()).generic_string();
            auto link = ("/download" / file.relative_path()).generic_string();

            res.fs_entries.push_back(dto::fs_entry{
                    file.filename(),
                    false,
                    is_directory,
                    path,
                    link});
        }

        for(auto& [job_name, job] : project->get_jobs())
        {
            auto it = std::find_if(
                    res.fs_entries.begin(),
                    res.fs_entries.end(),
                    [&job_name](auto& entry)
                    {
                        return entry.name == job_name;
                    });

            it->metainfo = dto::fs_entry::job_info{};
        }

        return tr.send(res);
    }

    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    dto::cis_project_error_doesnt_exist err;
    err.project = req.project;

    return tr.send_error(err, "Project doesn't exists.");
}

void get_job_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_job_info& req,
        transaction tr)
{
    auto* job = cis_manager.get_job_info(req.project, req.job);

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(job != nullptr && permitted)
    {
        dto::cis_job_info_success res;

        for(auto& file : job->get_files())
        {
            bool is_directory = file.dir_entry().is_directory();
            auto path = ("/" / file.relative_path()).generic_string();
            auto link = ("/download" / file.relative_path()).generic_string();

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

            auto it = std::find_if(
                    res.fs_entries.begin(),
                    res.fs_entries.end(),
                    [&build_name](auto& entry)
                    {
                        return entry.name == build_name;
                    });

            it->metainfo = dto::fs_entry::build_info{
                    info.status,
                    info.date};
        }

        return tr.send(res);
    }

    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    dto::cis_job_error_doesnt_exist err;
    err.project = req.project;
    err.job = req.job;

    return tr.send_error(err, "Job doesn't exists.");
}

void run_job(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_job_run& req,
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
                    dto::cis_job_error_invalid_params err;
                    
                    return tr.send_error(err, "Invalid params.");
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

        dto::cis_job_run_success res;

        return tr.send(res);
    }

    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    dto::cis_job_error_doesnt_exist err;
    err.project = req.project;
    err.job = req.job;

    return tr.send_error(err, "Job doesn't exists.");
}

void change_pass(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::user_auth_change_pass& req,
        transaction tr)
{
    bool ok = authentication_handler.change_pass(
            ctx.username,
            req.old_password,
            req.new_password);

    if(!ok)
    {
        dto::user_auth_error_pass_doesnt_match err;

        return tr.send_error(err, "Invalid password");
    }

    dto::user_auth_change_pass_success res;

    return tr.send(res);
}

void list_users(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::user_list& req,
        transaction tr)
{
    auto perm = rights.check_user_permission(ctx.username, "users.list");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        const auto users = authentication_handler.get_user_infos();

        dto::user_list_success res;

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
    
    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted");
}

void get_user_permissions(
        rights_manager& rights,
        request_context& ctx,
        const dto::user_permissions_get& req,
        transaction tr)
{
    auto perm = rights.check_user_permission(ctx.username, "users.permissions");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        const auto permissions = rights.get_permissions(req.username);

        dto::user_permissions_get_success res;

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

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted");
}

void set_user_permissions(
        rights_manager& rights,
        request_context& ctx,
        const dto::user_permissions_set& req,
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

        dto::user_permissions_set_success res;

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted");
}

void change_group(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::user_auth_change_group& req,
        transaction tr)
{

    if(!authentication_handler.has_user(req.username))
    {
        dto::user_auth_error_user_not_found err;

        return tr.send_error(err, "Invalid username.");
    }

    auto perm = rights.check_user_permission(ctx.username, "users.change_group");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        authentication_handler.change_group(req.username, req.group);

        dto::user_auth_change_group_success res;

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted.");
}

void ban_user(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::user_auth_ban& req,
        transaction tr)
{

    if(!authentication_handler.has_user(req.username))
    {
        dto::user_auth_error_user_not_found err;

        return tr.send_error(err, "Invalid username.");
    }

    auto perm = rights.check_user_permission(ctx.username, "users.change_group");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        authentication_handler.change_group(
                req.username,
                "disabled");

        dto::user_auth_ban_success res;

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted.");
}

void unban_user(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::user_auth_unban& req,
        transaction tr)
{

    if(!authentication_handler.has_user(req.username))
    {
        dto::user_auth_error_user_not_found err;

        return tr.send_error(err, "Invalid username.");
    }

    auto perm = rights.check_user_permission(ctx.username, "users.change_group");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        authentication_handler.change_group(
                req.username,
                "user");

        dto::user_auth_unban_success res;

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted.");
}

void generate_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::user_api_key_generate& req,
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

        dto::user_api_key_generate_success res;
        res.api_key = api_key.value();

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted.");
}

void get_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::user_api_key_get& req,
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

        dto::user_api_key_get_success res;
        res.api_key = api_key.value();

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted.");
}

void remove_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::user_api_key_remove& req,
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

        dto::user_api_key_remove_success res;

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted.");
}

void get_build_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_build_info& req,
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

        dto::cis_build_info_success res;
        res.status = info.status ? info.status.value() : -1;
        res.date = info.date ? info.date.value() : "";

        for(auto& file : build->get_files())
        {
            bool is_directory = file.dir_entry().is_directory();
            auto path = ("/" / file.relative_path()).generic_string();
            auto link = ("/download" / file.relative_path()).generic_string();

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
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    dto::cis_build_error_doesnt_exist err;
    err.project = req.project;
    err.job = req.job;
    err.build = req.build;

    return tr.send_error(err, "Build doesn't exists.");
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
                    path_it->string());
        }
    }

    return std::nullopt;
}

void refresh_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_refresh& req,
        transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, path);
            path_rights && !path_rights.value().read)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    auto refresh_result = cis_manager.refresh(path);

    if(!refresh_result)
    {
        dto::fs_entry_error_doesnt_exist err;

        return tr.send_error(err, "Path does not exists.");
    }

    dto::fs_entry_refresh_success res;

    return tr.send(res);
}

void remove_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_remove& req,
        transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, path);
            path_rights && !path_rights.value().write)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    auto remove_result = cis_manager.remove(path);

    if(!remove_result)
    {
        dto::fs_entry_error_doesnt_exist err;

        return tr.send_error(err, "Path does not exists.");
    }

    dto::fs_entry_remove_success res;

    return tr.send(res);
}

void move_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_move& req,
        transaction tr)
{

    std::filesystem::path old_path(req.old_path);
    std::filesystem::path new_path(req.new_path);

    if(!validate_path(old_path) || !validate_path(new_path))
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, old_path);
            path_rights && !path_rights.value().write)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, new_path);
            path_rights && !path_rights.value().write)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    std::error_code ec;
    fs.move_entry(old_path, new_path, ec);

    if(ec)
    {
        dto::fs_entry_error_cant_move err;

        return tr.send_error(err, "Error on move.");
    }

    dto::fs_entry_move_success res;

    return tr.send(res);
}

void new_directory(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_new_dir& req,
        transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, path);
            path_rights && !path_rights.value().write)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    std::error_code ec;
    fs.create_directory(path, ec);

    if(ec)
    {
        dto::fs_entry_error_cant_create_dir err;

        return tr.send_error(err, "Error while creating directory.");
    }

    dto::fs_entry_new_dir_success res;

    return tr.send(res);
}

void list_directory(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_list& req,
        transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, path);
            path_rights && !path_rights.value().read)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    if(auto it = fs.find(path); it != fs.end())
    {
        dto::fs_entry_list_success res;

        for(auto& file : it->childs())
        {
            bool is_directory = file.dir_entry().is_directory();
            auto path = ("/" / file.relative_path()).generic_string();
            auto link = ("/download" / file.relative_path()).generic_string();

            res.entries.push_back(dto::fs_entry{
                    file.filename(),
                    false,
                    is_directory,
                    path,
                    link});
        }

        return tr.send(res);
    }

    dto::fs_entry_error_doesnt_exist err;

    tr.send_error(err, "Path does not exists.");
}

void add_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_cron_add& req,
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

        dto::cis_cron_add_success res;

        return tr.send(res);
    }

    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    dto::cis_job_error_doesnt_exist err;
    
    err.project = req.project;
    err.job = req.job;

    return tr.send_error(err, "Job doesn't exists.");
}

void remove_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_cron_remove& req,
        transaction tr)
{
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

        dto::cis_cron_remove_success res;

        return tr.send(res);
    }

    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }
    
    dto::cis_job_error_doesnt_exist err;
    
    err.project = req.project;
    err.job = req.job;

    return tr.send_error(err, "Job doesn't exists.");
}

void list_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_cron_list& req,
        transaction tr)
{
    if(auto executor = tr.get_executor(); executor)
    {
        make_async_chain(executor.value())
            .then(cis_manager.list_cron(req.mask))
            .then([tr](const std::vector<cis::cron_entry>& entries)
                    {
                        dto::cis_cron_list_success res;
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
