/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#pragma once

#include <filesystem>

#include "websocket/dto/fs_entry.h"
#include "websocket/handlers/utils/wsh_log.h"
#include "cis/cis_structs_interface.h"

// WSHU is a websocket::handlers::utils namespace-like prefix

#define WSHU_CHECK_EC(ec) \
if(ec) \
{ \
    WSHU_LOG(scl::Level::Error, "Internal error: %s", ec.message()); \
    return tr.send_error("Internal error."); \
}

#define WSHU_CHECK_EC_MSG(ec, msg) \
if(ec) \
{ \
    WSHU_LOG(scl::Level::Error, "Internal error: %s", msg); \
    return tr.send_error(msg); \
}
