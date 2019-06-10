#pragma once

namespace websocket
{

enum class request_id
{
    auth_login_pass     = 1,
    auth_token          = 3,
    logout              = 5,
    change_pass         = 7,
    list_users          = 9,
    get_user_permissions= 11,
    set_user_permissions= 13,
    change_group        = 15,
    disable_user        = 17,
    generate_api_key    = 19,
    get_api_key         = 21,
    remove_api_key      = 23,
    list_projects       = 65,
    get_project_info    = 67,
    get_job_info        = 69,
    run_job             = 71,
    get_build_info      = 75,
    refresh_fs_entry    = 77,
    remove_fs_entry     = 79,
    move_fs_entry       = 81,
    new_directory       = 83,
    list_directory      = 85,
    add_cis_cron        = 87,
    remove_cis_cron     = 89,
};

enum class response_id
{
    generic_error       = -1,
    reserved            = 0,
    auth_login_pass     = 2,
    auth_token          = 4,
    logout              = 6,
    change_pass         = 8,
    list_users          = 10,
    get_user_permissions= 12,
    set_user_permissions= 14,
    change_group        = 16,
    disable_user        = 18,
    generate_api_key    = 20,
    get_api_key         = 22,
    remove_api_key      = 24,
    list_projects       = 66,
    get_project_info    = 68,
    get_job_info        = 70,
    run_job             = 72,
    get_build_info      = 76,
    refresh_fs_entry    = 78,
    remove_fs_entry     = 80,
    move_fs_entry       = 82,
    new_directory       = 84,
    list_directory      = 86,
    add_cis_cron        = 88,
    remove_cis_cron     = 90,
};

} // namespace websocket
