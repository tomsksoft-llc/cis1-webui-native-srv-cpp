/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#pragma once

#include "logger.h"

using namespace std::string_literals;

// WSHU is a websocket::handlers::utils namespace-like prefix

constexpr const char* __file_name__(const char* path)
{
    const char* file = path;
    while (*path)
    {
        if (*path++ == '/')
        {
            file = path;
        }
    }
    return file;
}

#define WSHU_LOG(level, format, ...) \
__FUNCTION__ == "operator()"s \
? ex_log(level, Protocol::WS, __file_name__(__FILE__), ctx, SCFormat(format, ##__VA_ARGS__)) \
: ex_log(level, Protocol::WS, __FUNCTION__, ctx, SCFormat(format, ##__VA_ARGS__))
