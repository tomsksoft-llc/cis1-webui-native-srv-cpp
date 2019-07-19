#include "dto_to_event_id.h"

namespace json
{

template <>
int32_t dto_to_event_id<ws::dto::auth_login_pass_response>()
{
    return 2;
}

template <>
int32_t dto_to_event_id<ws::dto::auth_token_response>()
{
    return 4;
}

template <>
int32_t dto_to_event_id<ws::dto::logout_response>()
{
    return 6;
}

template <>
int32_t dto_to_event_id<ws::dto::change_pass_response>()
{
    return 8;
}

template <>
int32_t dto_to_event_id<ws::dto::get_user_list_response>()
{
    return 10;
}

template <>
int32_t dto_to_event_id<ws::dto::get_user_permissions_response>()
{
    return 12;
}

template <>
int32_t dto_to_event_id<ws::dto::set_user_permissions_response>()
{
    return 14;
}

template <>
int32_t dto_to_event_id<ws::dto::change_group_response>()
{
    return 16;
}

template <>
int32_t dto_to_event_id<ws::dto::disable_user_response>()
{
    return 18;
}

template <>
int32_t dto_to_event_id<ws::dto::generate_api_key_response>()
{
    return 20;
}

template <>
int32_t dto_to_event_id<ws::dto::get_api_key_response>()
{
    return 22;
}

template <>
int32_t dto_to_event_id<ws::dto::remove_api_key_response>()
{
    return 24;
}

template <>
int32_t dto_to_event_id<ws::dto::get_project_list_response>()
{
    return 66;
}

template <>
int32_t dto_to_event_id<ws::dto::get_project_info_response>()
{
    return 68;
}

template <>
int32_t dto_to_event_id<ws::dto::get_job_info_response>()
{
    return 70;
}

template <>
int32_t dto_to_event_id<ws::dto::run_job_response>()
{
    return 72;
}

template <>
int32_t dto_to_event_id<ws::dto::get_build_info_response>()
{
    return 76;
}

template <>
int32_t dto_to_event_id<ws::dto::refresh_fs_entry_response>()
{
    return 78;
}

template <>
int32_t dto_to_event_id<ws::dto::remove_fs_entry_response>()
{
    return 80;
}

template <>
int32_t dto_to_event_id<ws::dto::move_fs_entry_response>()
{
    return 82;
}

template <>
int32_t dto_to_event_id<ws::dto::new_directory_response>()
{
    return 84;
}

template <>
int32_t dto_to_event_id<ws::dto::list_directory_response>()
{
    return 86;
}

template <>
int32_t dto_to_event_id<ws::dto::add_cis_cron_response>()
{
    return 88;
}

template <>
int32_t dto_to_event_id<ws::dto::remove_cis_cron_response>()
{
    return 90;
}

template <>
int32_t dto_to_event_id<ws::dto::list_cis_cron_response>()
{
    return 92;
}

} // namespace json
