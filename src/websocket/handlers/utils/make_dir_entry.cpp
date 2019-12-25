/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/utils/make_dir_entry.h"

namespace websocket
{

namespace handlers
{

dto::fs_entry make_dir_entry(
        const std::filesystem::path& root,
        const fs_entry_interface& entry)
{
    dto::fs_entry res;

    make_dir_entry(res, root, entry);

    return res;
}

} // handlers

} // websocket
