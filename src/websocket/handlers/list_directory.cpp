/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/list_directory.h"

#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/fs_entry_error_doesnt_exist.h"
#include "websocket/dto/fs_entry_list_success.h"

#include "path_utils.h"
#include "cis/cis_structs.h"
#include "websocket/handlers/utils/make_dir_entry.h"

namespace websocket
{

namespace handlers
{

void list_directory(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::fs_entry_list& req,
        cis1::proto_utils::transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    std::error_code ec;

    auto path_rights = get_path_rights(ctx, rights, path, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(!path_rights || !path_rights.value().read)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    if(auto it = fs.find(path); it != fs.end())
    {
        dto::fs_entry_list_success res;

        for(auto itt = it.begin(); itt != it.end(); ++itt)
        {
            cis::fs_entry_ref entry(itt);

            res.entries.push_back(
                    make_dir_entry(fs.root().path(), entry));
        }

        return tr.send(res);
    }

    dto::fs_entry_error_doesnt_exist err;

    tr.send_error(err, "Path does not exists.");
}

} // namespace handlers

} // namespace websocket
