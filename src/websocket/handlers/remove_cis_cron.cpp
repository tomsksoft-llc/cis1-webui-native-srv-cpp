#include "websocket/handlers/remove_cis_cron.h"

#include "websocket/dto/cis_job_error_doesnt_exist.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/cis_cron_remove_success.h"

namespace websocket
{

namespace handlers
{

void remove_cis_cron(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_cron_remove& req,
        cis1::proto_utils::transaction tr)
{
    auto* job = cis_manager.get_job_info(req.project, req.job);
    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted =
        perm.has_value() ? (perm.value().execute && perm.value().write) : true;

    if(job != nullptr && permitted)
    {
        cis_manager.remove_cron(
                req.project,
                req.job,
                req.cron_expr);

        dto::cis_cron_remove_success res;

        return tr.send(res);
    }

    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    dto::cis_job_error_doesnt_exist err;

    err.project = req.project;
    err.job = req.job;

    return tr.send_error(err, "Job doesn't exists.");
}

} // namespace handlers

} // namespace websocket
