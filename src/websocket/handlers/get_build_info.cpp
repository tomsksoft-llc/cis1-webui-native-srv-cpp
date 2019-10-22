#include "websocket/handlers/get_build_info.h"

#include "websocket/dto/cis_build_info_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/cis_build_error_doesnt_exist.h"

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
    auto* build = cis_manager.get_build_info(
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

        for(auto& file : build->get_files())
        {
            bool is_directory = file.dir_entry().is_directory();
            auto path = ("/" / file.relative_path()).generic_string();
            auto link = ("/download" / file.relative_path()).generic_string();

            res.fs_entries.push_back(dto::fs_entry{
                    file.filename(),
                    false,
                    is_directory,
                    path,
                    link});
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
