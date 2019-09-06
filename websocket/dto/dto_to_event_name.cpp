#include "dto_to_event_name.h"

namespace json
{

template <>
std::string dto_to_event_name<ws::dto::auth_login_pass>()
{
    return "auth.login_pass";
}

template <>
std::string dto_to_event_name<ws::dto::auth_login_pass_success>()
{
    return "auth.success";
}

template <>
std::string dto_to_event_name<ws::dto::auth_error_wrong_credentials>()
{
    return "auth.error.wrong_credentials";
}

template <>
std::string dto_to_event_name<ws::dto::auth_token>()
{
    return "auth.token";
}

template <>
std::string dto_to_event_name<ws::dto::auth_logout>()
{
    return "auth.logout";
}

template <>
std::string dto_to_event_name<ws::dto::auth_logout_success>()
{
    return "auth.logout_success";
}

template <>
std::string dto_to_event_name<ws::dto::user_auth_change_pass>()
{
    return "user.auth.change_pass";
}

template <>
std::string dto_to_event_name<ws::dto::user_auth_change_pass_success>()
{
    return "user.auth.change_pass.success";
}

template <>
std::string dto_to_event_name<ws::dto::user_auth_error_pass_doesnt_match>()
{
    return "user.auth.error.pass_doesnt_match";
}

template <>
std::string dto_to_event_name<ws::dto::user_auth_error_user_not_found>()
{
    return "user.auth.error.user_not_found";
}

template <>
std::string dto_to_event_name<ws::dto::user_list>()
{
    return "user.list";
}

template <>
std::string dto_to_event_name<ws::dto::user_list_success>()
{
    return "user.list.success";
}

template <>
std::string dto_to_event_name<ws::dto::user_permissions_get>()
{
    return "user.permissions.get";
}

template <>
std::string dto_to_event_name<ws::dto::user_permissions_get_success>()
{
    return "user.permissions.get.success";
}

template <>
std::string dto_to_event_name<ws::dto::user_permissions_set>()
{
    return "user.permissions.set";
}

template <>
std::string dto_to_event_name<ws::dto::user_permissions_set_success>()
{
    return "user.permissions.set.success";
}

template <>
std::string dto_to_event_name<ws::dto::user_permissions_error_access_denied>()
{
    return "user.permissions.error.access_denied";
}

template <>
std::string dto_to_event_name<ws::dto::user_auth_change_group>()
{
    return "user.auth.change_group";
}

template <>
std::string dto_to_event_name<ws::dto::user_auth_change_group_success>()
{
    return "user.auth.change_group.success";
}

template <>
std::string dto_to_event_name<ws::dto::user_auth_ban>()
{
    return "user.auth.ban";
}

template <>
std::string dto_to_event_name<ws::dto::user_auth_unban>()
{
    return "user.auth.unban";
}

template <>
std::string dto_to_event_name<ws::dto::user_auth_ban_success>()
{
    return "user.auth.ban.success";
}

template <>
std::string dto_to_event_name<ws::dto::user_auth_unban_success>()
{
    return "user.auth.unban.success";
}

template <>
std::string dto_to_event_name<ws::dto::user_api_key_generate>()
{
    return "user.api.key.generate";
}

template <>
std::string dto_to_event_name<ws::dto::user_api_key_generate_success>()
{
    return "user.api.key.generate.success";
}

template <>
std::string dto_to_event_name<ws::dto::user_api_key_get>()
{
    return "user.api.key.get";
}

template <>
std::string dto_to_event_name<ws::dto::user_api_key_get_success>()
{
    return "user.api.key.get.success";
}

template <>
std::string dto_to_event_name<ws::dto::user_api_key_remove>()
{
    return "user.api.key.remove";
}

template <>
std::string dto_to_event_name<ws::dto::user_api_key_remove_success>()
{
    return "user.api.key.remove.success";
}

template <>
std::string dto_to_event_name<ws::dto::cis_project_list_get>()
{
    return "cis.project_list.get";
}

template <>
std::string dto_to_event_name<ws::dto::cis_project_list_get_success>()
{
    return "cis.project_list.get.success";
}

template <>
std::string dto_to_event_name<ws::dto::cis_project_info>()
{
    return "cis.project.info";
}

template <>
std::string dto_to_event_name<ws::dto::cis_project_info_success>()
{
    return "cis.project.info.success";
}

template <>
std::string dto_to_event_name<ws::dto::cis_project_error_doesnt_exist>()
{
    return "cis.project.error.doesnt_exist";
}

template <>
std::string dto_to_event_name<ws::dto::cis_job_info>()
{
    return "cis.job.info";
}

template <>
std::string dto_to_event_name<ws::dto::cis_job_info_success>()
{
    return "cis.job.info.success";
}

template <>
std::string dto_to_event_name<ws::dto::cis_job_error_doesnt_exist>()
{
    return "cis.job.error.doesnt_exist";
}

template <>
std::string dto_to_event_name<ws::dto::cis_job_error_invalid_params>()
{
    return "cis.job.error.invalid_params";
}

template <>
std::string dto_to_event_name<ws::dto::cis_job_run>()
{
    return "cis.job.run";
}

template <>
std::string dto_to_event_name<ws::dto::cis_job_run_success>()
{
    return "cis.job.run.success";
}

template <>
std::string dto_to_event_name<ws::dto::cis_build_info>()
{
    return "cis.build.info";
}

template <>
std::string dto_to_event_name<ws::dto::cis_build_info_success>()
{
    return "cis.build.info.success";
}

template <>
std::string dto_to_event_name<ws::dto::cis_build_error_doesnt_exist>()
{
    return "cis.build.error.doesnt_exist";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_refresh>()
{
    return "fs.entry.refresh";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_refresh_success>()
{
    return "fs.entry.refresh.success";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_remove>()
{
    return "fs.entry.remove";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_remove_success>()
{
    return "fs.entry.remove.success";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_move>()
{
    return "fs.entry.move";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_move_success>()
{
    return "fs.entry.move.success";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_new_dir>()
{
    return "fs.entry.new_dir";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_new_dir_success>()
{
    return "fs.entry.new_dir.success";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_list>()
{
    return "fs.entry.list";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_list_success>()
{
    return "fs.entry.list.success";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_error_invalid_path>()
{
    return "fs.entry.error.invalid_path";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_error_doesnt_exist>()
{
    return "fs.entry.error.doesnt_exist";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_error_cant_move>()
{
    return "fs.entry.error.cant_move";
}

template <>
std::string dto_to_event_name<ws::dto::fs_entry_error_cant_create_dir>()
{
    return "fs.entry.error.cant_create_dir";
}

template <>
std::string dto_to_event_name<ws::dto::cis_cron_add>()
{
    return "cis.cron.add";
}

template <>
std::string dto_to_event_name<ws::dto::cis_cron_add_success>()
{
    return "cis.cron.add.success";
}

template <>
std::string dto_to_event_name<ws::dto::cis_cron_remove>()
{
    return "cis.cron.remove";
}

template <>
std::string dto_to_event_name<ws::dto::cis_cron_remove_success>()
{
    return "cis.cron.remove.success";
}

template <>
std::string dto_to_event_name<ws::dto::cis_cron_list>()
{
    return "cis.cron.list";
}

template <>
std::string dto_to_event_name<ws::dto::cis_cron_list_success>()
{
    return "cis.cron.list.success";
}

} // namespace json
