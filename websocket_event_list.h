#pragma once

enum class ws_request_id
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
    list_projects       = 65,
    list_jobs           = 67,
    get_job_info        = 69,
    run_job             = 71,
    rename_job          = 73,
    get_build_info      = 75,
};

enum class ws_response_id
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
    list_projects       = 66,
    list_jobs           = 67,
    get_job_info        = 70,
    run_job             = 72,
    rename_job          = 74,
    get_build_info      = 76,
};
