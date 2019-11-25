#include "websocket/handlers/remove_cis_project.h"

#include "websocket/dto/cis_project_remove_success.h"
#include "websocket/dto/cis_project_error_doesnt_exist.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void remove_cis_project(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_project_remove& req,
        cis1::proto_utils::transaction tr)
{
    auto project = cis_manager.get_project_info(req.project);

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(project != nullptr && permitted)
    {
        auto project_path =
                std::filesystem::path{"/"} / req.project;

        auto& fs = cis_manager.fs();

        if(auto it = fs.find(project_path); it != fs.end())
        {
            it.remove();

            dto::cis_project_remove_success res;

            return tr.send(res);
        }

        dto::cis_project_error_doesnt_exist err;
        err.project = req.project;

        return tr.send_error(err, "Project doesn't exists.");
    }

    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    dto::cis_project_error_doesnt_exist err;
    err.project = req.project;

    return tr.send_error(err, "Project doesn't exists.");
}

} // namespace handlers

} // namespace websocket