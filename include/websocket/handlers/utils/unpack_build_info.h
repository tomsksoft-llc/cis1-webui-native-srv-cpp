/*
*    TomskSoft CIS1 WebUI
*
*   (c) 2020 TomskSoft LLC
*   (c) Mokin Innokentiy [mia@tomsksoft.com]
*
*/

#pragma once

#include "cis/cis_structs_interface.h"

namespace websocket
{

namespace handlers
{

namespace utils
{

template <class T>
T unpack_build_info(const build_interface::info& info)
{
    return T{
            info.status
                ? (info.status.value() == 0
                        ? "success"
                        : "failed")
                : "in_progress",
            info.status,
            info.session_id,
            info.date};
}

} // namespace utils

} // namespace handlers

} // namesapce websocket
