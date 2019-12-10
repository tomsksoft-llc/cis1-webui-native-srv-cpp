#include "websocket/handlers/add_cis_project.h"

#include "websocket/dto/cis_project_add_success.h"
#include "websocket/dto/cis_project_error_already_exist.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void add_cis_project(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_project_add& req,
        cis1::proto_utils::transaction tr)
{

    auto project = cis_manager.get_project_info(req.project);

    if(project != nullptr)
    {
        dto::cis_project_error_already_exist err;
        err.project = req.project;

        return tr.send_error(err, "Project already exists.");
    }

    std::error_code ec;

    cis_manager.create_project(req.project, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    dto::cis_project_add_success res;

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
