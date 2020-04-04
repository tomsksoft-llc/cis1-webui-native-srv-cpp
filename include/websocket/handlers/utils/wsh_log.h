/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#pragma once

#include "logger.h"

// WSHU is a websocket::handlers::utils namespace-like prefix

#define WSHU_LOG(level, format, ...) \
ex_log(level, Protocol::WS, __FUNCTION__, ctx, SCFormat(format, ##__VA_ARGS__))
