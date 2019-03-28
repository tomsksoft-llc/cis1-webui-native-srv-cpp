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
    make_admin          = 15,
    disable_user        = 17,
    generate_api_key    = 19,
    list_projects       = 65,
    list_jobs           = 67,
    list_builds         = 69,
    run_job             = 71,
    rename_job          = 73,
    get_job_file        = 75,
    set_job_file        = 77,
    get_build_info      = 79,
    get_build_artifact  = 81
};

enum class ws_response_id
{
    generic_error       = -1,
    reserved            = 0,
    auth_login_pass     = 1,
    auth_token          = 3,
    logout              = 5,
    change_pass         = 7,
    list_users          = 9,
    get_user_permissions= 11,
    set_user_permissions= 13,
    make_admin          = 15,
    disable_user        = 17,
    generate_api_key    = 19,
    list_projects       = 65,
    list_jobs           = 67,
    list_builds         = 69,
    run_job             = 71,
    rename_job          = 73,
    get_job_file        = 75,
    set_job_file        = 77,
    get_build_info      = 79,
    get_build_artifact  = 81
};
