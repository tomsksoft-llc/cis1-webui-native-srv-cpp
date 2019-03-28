#pragma once

enum class ws_request_id
{
    auth_login_pass = 1,
    auth_token      = 3,
    auth_logout     = 5,
    projects_list   = 21,
    jobs_list       = 23,
    builds_list     = 25,
    run_job         = 27
};

enum class ws_response_id
{
    common_error    = -1,
    auth_login_pass = 2,
    auth_token      = 4,
    auth_logout     = 6,
    projects_list   = 22,
    jobs_list       = 24,
    builds_list     = 26,
    run_job         = 28
};
