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
#include "cis/cis_structs_interface.h"

// WSHU is a websocket::handlers::utils namespace-like prefix
// TODO add logging

#define WSHU_CHECK_EC(ec) if(ec) { return tr.send_error("Internal error."); }

#define WSHU_CHECK_EC_MSG(ec, msg) if(ec) { return tr.send_error(msg); }
