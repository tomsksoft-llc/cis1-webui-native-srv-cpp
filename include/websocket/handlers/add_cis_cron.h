#pragma once

#include "request_context.h"
#include "rights_manager_interface.h"
#include "cis/cis_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/cis_cron_add.h"

#include "websocket/dto/cis_cron_add_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/cis_job_error_doesnt_exist.h"

#include "cron_utils.h"

namespace websocket
{

namespace handlers
{

template <class Transaction>
void add_cis_cron(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_cron_add& req,
        Transaction&& tr)
{
    transaction_handle handle{std::forward<Transaction>(tr)};

    if(!cron::validate_expr(req.cron_expr))
    {
        return handle.send_error("Invalid cron expression.");
    }

    auto* job = cis_manager.get_job_info(req.project, req.job);
    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted =
        perm.has_value() ? (perm.value().execute && perm.value().write) : true;

    if(job != nullptr && permitted)
    {
        cis_manager.add_cron(
                req.project,
                req.job,
                req.cron_expr);

        dto::cis_cron_add_success res;

        return handle.send(res);
    }

    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;

        return handle.send_error(err, "Action not permitted.");
    }

    dto::cis_job_error_doesnt_exist err;

    err.project = req.project;
    err.job = req.job;

    return handle.send_error(err, "Job doesn't exists.");
}

} // namespace handlers

} // namespace websocket