#pragma once

#include <string>

#include "auth_login_pass.h"
#include "auth_login_pass_success.h"
#include "auth_error_wrong_credentials.h"
#include "auth_token.h"
#include "auth_logout.h"
#include "auth_logout_success.h"
#include "user_auth_change_pass.h"
#include "user_auth_change_pass_success.h"
#include "user_auth_error_pass_doesnt_match.h"
#include "user_auth_error_user_not_found.h"
#include "user_list.h"
#include "user_list_success.h"
#include "user_permissions_get.h"
#include "user_permissions_get_success.h"
#include "user_permissions_set.h"
#include "user_permissions_set_success.h"
#include "user_permissions_error_access_denied.h"
#include "user_auth_change_group.h"
#include "user_auth_change_group_success.h"
#include "user_auth_ban.h"
#include "user_auth_unban.h"
#include "user_auth_ban_success.h"
#include "user_auth_unban_success.h"
#include "user_api_key_generate.h"
#include "user_api_key_generate_success.h"
#include "user_api_key_get.h"
#include "user_api_key_get_success.h"
#include "user_api_key_remove.h"
#include "user_api_key_remove_success.h"
#include "cis_project_list_get.h"
#include "cis_project_list_get_success.h"
#include "cis_project_info.h"
#include "cis_project_info_success.h"
#include "cis_project_error_doesnt_exist.h"
#include "cis_job_info.h"
#include "cis_job_info_success.h"
#include "cis_job_error_doesnt_exist.h"
#include "cis_job_error_invalid_params.h"
#include "cis_job_run.h"
#include "cis_job_run_success.h"
#include "cis_build_info.h"
#include "cis_build_info_success.h"
#include "cis_build_error_doesnt_exist.h"
#include "fs_entry_refresh.h"
#include "fs_entry_refresh_success.h"
#include "fs_entry_remove.h"
#include "fs_entry_remove_success.h"
#include "fs_entry_move.h"
#include "fs_entry_move_success.h"
#include "fs_entry_new_dir.h"
#include "fs_entry_new_dir_success.h"
#include "fs_entry_list.h"
#include "fs_entry_list_success.h"
#include "fs_entry_error_invalid_path.h"
#include "fs_entry_error_doesnt_exist.h"
#include "fs_entry_error_cant_move.h"
#include "fs_entry_error_cant_create_dir.h"
#include "cis_cron_add.h"
#include "cis_cron_add_success.h"
#include "cis_cron_remove.h"
#include "cis_cron_remove_success.h"
#include "cis_cron_list.h"
#include "cis_cron_list_success.h"

namespace json
{

namespace ws = websocket;

template <class ResType>
std::string dto_to_event_name();

template <>
std::string dto_to_event_name<ws::dto::auth_login_pass>();

template <>
std::string dto_to_event_name<ws::dto::auth_login_pass_success>();

template <>
std::string dto_to_event_name<ws::dto::auth_error_wrong_credentials>();

template <>
std::string dto_to_event_name<ws::dto::auth_token>();

template <>
std::string dto_to_event_name<ws::dto::auth_logout>();

template <>
std::string dto_to_event_name<ws::dto::auth_logout_success>();

template <>
std::string dto_to_event_name<ws::dto::user_auth_change_pass>();

template <>
std::string dto_to_event_name<ws::dto::user_auth_change_pass_success>();

template <>
std::string dto_to_event_name<ws::dto::user_auth_error_pass_doesnt_match>();

template <>
std::string dto_to_event_name<ws::dto::user_auth_error_user_not_found>();

template <>
std::string dto_to_event_name<ws::dto::user_list>();

template <>
std::string dto_to_event_name<ws::dto::user_list_success>();

template <>
std::string dto_to_event_name<ws::dto::user_permissions_get>();

template <>
std::string dto_to_event_name<ws::dto::user_permissions_get_success>();

template <>
std::string dto_to_event_name<ws::dto::user_permissions_set>();

template <>
std::string dto_to_event_name<ws::dto::user_permissions_set_success>();

template <>
std::string dto_to_event_name<ws::dto::user_permissions_error_access_denied>();

template <>
std::string dto_to_event_name<ws::dto::user_auth_change_group>();

template <>
std::string dto_to_event_name<ws::dto::user_auth_change_group_success>();

template <>
std::string dto_to_event_name<ws::dto::user_auth_ban>();

template <>
std::string dto_to_event_name<ws::dto::user_auth_unban>();

template <>
std::string dto_to_event_name<ws::dto::user_auth_ban_success>();

template <>
std::string dto_to_event_name<ws::dto::user_auth_unban_success>();

template <>
std::string dto_to_event_name<ws::dto::user_api_key_generate>();

template <>
std::string dto_to_event_name<ws::dto::user_api_key_generate_success>();

template <>
std::string dto_to_event_name<ws::dto::user_api_key_get>();

template <>
std::string dto_to_event_name<ws::dto::user_api_key_get_success>();

template <>
std::string dto_to_event_name<ws::dto::user_api_key_remove>();

template <>
std::string dto_to_event_name<ws::dto::user_api_key_remove_success>();

template <>
std::string dto_to_event_name<ws::dto::cis_project_list_get>();

template <>
std::string dto_to_event_name<ws::dto::cis_project_list_get_success>();

template <>
std::string dto_to_event_name<ws::dto::cis_project_info>();

template <>
std::string dto_to_event_name<ws::dto::cis_project_error_doesnt_exist>();

template <>
std::string dto_to_event_name<ws::dto::cis_project_info_success>();

template <>
std::string dto_to_event_name<ws::dto::cis_job_info>();

template <>
std::string dto_to_event_name<ws::dto::cis_job_info_success>();

template <>
std::string dto_to_event_name<ws::dto::cis_job_error_doesnt_exist>();

template <>
std::string dto_to_event_name<ws::dto::cis_job_error_invalid_params>();

template <>
std::string dto_to_event_name<ws::dto::cis_job_run>();

template <>
std::string dto_to_event_name<ws::dto::cis_job_run_success>();

template <>
std::string dto_to_event_name<ws::dto::cis_build_info>();

template <>
std::string dto_to_event_name<ws::dto::cis_build_info_success>();

template <>
std::string dto_to_event_name<ws::dto::cis_build_error_doesnt_exist>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_refresh>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_refresh_success>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_remove>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_remove_success>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_move>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_move_success>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_new_dir>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_new_dir_success>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_list>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_list_success>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_error_invalid_path>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_error_doesnt_exist>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_error_cant_move>();

template <>
std::string dto_to_event_name<ws::dto::fs_entry_error_cant_create_dir>();

template <>
std::string dto_to_event_name<ws::dto::cis_cron_add>();

template <>
std::string dto_to_event_name<ws::dto::cis_cron_add_success>();

template <>
std::string dto_to_event_name<ws::dto::cis_cron_remove>();

template <>
std::string dto_to_event_name<ws::dto::cis_cron_remove_success>();

template <>
std::string dto_to_event_name<ws::dto::cis_cron_list>();

template <>
std::string dto_to_event_name<ws::dto::cis_cron_list_success>();

} //namespace json
