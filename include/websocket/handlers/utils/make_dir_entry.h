/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <filesystem>

#include "websocket/dto/fs_entry.h"
#include "cis/cis_structs_interface.h"

namespace websocket
{

namespace handlers
{

dto::fs_entry make_dir_entry(
        const std::filesystem::path& root,
        const fs_entry_interface& entry);

} // handlers

} // websocket
