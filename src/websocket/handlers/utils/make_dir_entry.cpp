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

    auto& file = entry.dir_entry();

    res.name = file.path().filename();

    auto relative_path = file.path().lexically_relative(
            root);

    auto path = ("/" / relative_path)
            .generic_string();

    res.path = path;

    res.link = "/download" + path;

    if(file.is_directory())
    {
        res.metainfo = dto::fs_entry::directory_info{};
    }
    else if(file.is_regular_file())
    {
        std::error_code ec;
        auto status = std::filesystem::status(
                file.path(), ec);


        bool executable = ec ? false :
            (status.permissions() & std::filesystem::perms::owner_exec)
            != std::filesystem::perms::none;

        res.metainfo = dto::fs_entry::file_info{executable};
    }

    return res;
}

} // handlers

} // websocket
