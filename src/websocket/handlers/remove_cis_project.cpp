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

    std::error_code ec;

    auto perm = rights.check_project_right(ctx.username, req.project, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    auto permitted = perm.has_value() ? perm.value().read : true;

    if(project != nullptr && permitted)
    {
        std::error_code ec;

        cis_manager.remove_project(project, ec);

        if(ec)
        {
            return tr.send_error("Internal error.");
        }

        dto::cis_project_remove_success res;

        return tr.send(res);
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
