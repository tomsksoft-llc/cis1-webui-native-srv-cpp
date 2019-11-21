#include "websocket/handlers/get_build_info.h"

#include "websocket/dto/cis_build_info_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/cis_build_error_doesnt_exist.h"
#include "websocket/handlers/utils/make_dir_entry.h"
#include "cis/cis_structs.h"

namespace websocket
{

namespace handlers
{

void get_build_info(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_build_info& req,
        cis1::proto_utils::transaction tr)
{
    auto build = cis_manager.get_build_info(
            req.project,
            req.job,
            req.build);

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().write : true;

    if(build != nullptr && permitted)
    {
        auto& info = build->get_info();

        dto::cis_build_info_success res;
        res.status = info.status ? info.status.value() : -1;
        res.date = info.date ? info.date.value() : "";

        for(auto it  = build->get_files().begin();
                 it != build->get_files().end();
                 ++it)
        {
            cis::fs_entry_ref entry(it);
            res.fs_entries.push_back(
                    make_dir_entry(
                            cis_manager.fs().root().path(),
                            entry));
        }

        return tr.send(res);
    }

    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    dto::cis_build_error_doesnt_exist err;
    err.project = req.project;
    err.job = req.job;
    err.build = req.build;

    return tr.send_error(err, "Build doesn't exists.");
}

} // namespace handlers

} // namespace websocket
