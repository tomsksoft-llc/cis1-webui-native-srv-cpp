#pragma once

#include "auth_login_pass_response.h"
#include "auth_token_response.h"
#include "logout_response.h"
#include "change_pass_response.h"
#include "get_user_list_response.h"
#include "get_user_permissions_response.h"
#include "set_user_permissions_response.h"
#include "change_group_response.h"
#include "disable_user_response.h"
#include "generate_api_key_response.h"
#include "get_api_key_response.h"
#include "remove_api_key_response.h"
#include "get_project_list_response.h"
#include "get_project_info_response.h"
#include "get_job_info_response.h"
#include "run_job_response.h"
#include "get_build_info_response.h"
#include "refresh_fs_entry_response.h"
#include "remove_fs_entry_response.h"
#include "move_fs_entry_response.h"
#include "new_directory_response.h"
#include "list_directory_response.h"
#include "add_cis_cron_response.h"
#include "remove_cis_cron_response.h"
#include "list_cis_cron_response.h"

namespace json
{

namespace ws = websocket;

template <class ResType>
int32_t dto_to_event_id();

template <>
int32_t dto_to_event_id<ws::dto::auth_login_pass_response>();

template <>
int32_t dto_to_event_id<ws::dto::auth_token_response>();

template <>
int32_t dto_to_event_id<ws::dto::logout_response>();

template <>
int32_t dto_to_event_id<ws::dto::change_pass_response>();

template <>
int32_t dto_to_event_id<ws::dto::get_user_list_response>();

template <>
int32_t dto_to_event_id<ws::dto::get_user_permissions_response>();

template <>
int32_t dto_to_event_id<ws::dto::set_user_permissions_response>();

template <>
int32_t dto_to_event_id<ws::dto::change_group_response>();

template <>
int32_t dto_to_event_id<ws::dto::disable_user_response>();

template <>
int32_t dto_to_event_id<ws::dto::generate_api_key_response>();

template <>
int32_t dto_to_event_id<ws::dto::get_api_key_response>();

template <>
int32_t dto_to_event_id<ws::dto::remove_api_key_response>();

template <>
int32_t dto_to_event_id<ws::dto::get_project_list_response>();

template <>
int32_t dto_to_event_id<ws::dto::get_project_info_response>();

template <>
int32_t dto_to_event_id<ws::dto::get_job_info_response>();

template <>
int32_t dto_to_event_id<ws::dto::run_job_response>();

template <>
int32_t dto_to_event_id<ws::dto::get_build_info_response>();

template <>
int32_t dto_to_event_id<ws::dto::refresh_fs_entry_response>();

template <>
int32_t dto_to_event_id<ws::dto::remove_fs_entry_response>();

template <>
int32_t dto_to_event_id<ws::dto::move_fs_entry_response>();

template <>
int32_t dto_to_event_id<ws::dto::new_directory_response>();

template <>
int32_t dto_to_event_id<ws::dto::list_directory_response>();

template <>
int32_t dto_to_event_id<ws::dto::add_cis_cron_response>();

template <>
int32_t dto_to_event_id<ws::dto::remove_cis_cron_response>();

template <>
int32_t dto_to_event_id<ws::dto::list_cis_cron_response>();

} //namespace json
