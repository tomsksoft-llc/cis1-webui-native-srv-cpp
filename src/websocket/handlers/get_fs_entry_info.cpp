/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_fs_entry_info.h"

#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/fs_entry_error_doesnt_exist.h"
#include "websocket/dto/fs_entry_info_success.h"

#include "path_utils.h"

namespace websocket
{

namespace handlers
{

void get_fs_entry_info(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::fs_entry_info& req,
        cis1::proto_utils::transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, path);
            path_rights && !path_rights.value().read)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    if(auto it = fs.find(path); it != fs.end())
    {
        dto::fs_entry_info_success res;

        auto& file = *it;

        res.name = file.path().filename();

        auto relative_path = file.path().lexically_relative(
                cis_manager.fs().root().path());

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
            res.metainfo = dto::fs_entry::file_info{};
        }

        return tr.send(res);
    }

    dto::fs_entry_error_doesnt_exist err;

    tr.send_error(err, "Path does not exists.");
}

} // namespace handlers

} // namespace websocket
