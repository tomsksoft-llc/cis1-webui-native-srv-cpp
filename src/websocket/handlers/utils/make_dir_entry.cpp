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
    auto& file = entry.dir_entry();

    bool is_directory = file.is_directory();

    auto relative_path = file.path().lexically_relative(root);

    auto path = ("/" / relative_path)
            .generic_string();

    auto link = "/download" + path;

    return dto::fs_entry{
            file.path().filename(),
            false,
            is_directory,
            path,
            link};
}

} // handlers

} // websocket
