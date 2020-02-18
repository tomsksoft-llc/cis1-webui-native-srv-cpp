/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <cis1_proto_utils/transaction.h>

#include "request_context.h"
#include "rights_manager_interface.h"
#include "cis/cis_manager.h"

#include "websocket/dto/cis_cron_add.h"

#include "websocket/dto/cis_cron_add_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/cis_job_error_doesnt_exist.h"
#include "websocket/dto/user_error_login_required.h"

#include "websocket/transaction_handle.h"
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

    auto job = cis_manager.get_job_info(req.project, req.job);

    std::error_code ec;

    auto perm = rights.check_project_right(ctx.client_info, req.project, ec);

    if(ec)
    {
        return handle.send_error("Internal error.");
    }

    auto permitted = perm && perm.value().write && perm.value().execute;

    if(job != nullptr && permitted)
    {
        if(auto executor = handle.get_executor(); executor)
        {
            make_async_chain(executor.value())
                .then(cis_manager.add_cron(
                        req.project,
                        req.job,
                        req.cron_expr))
                .then([handle = std::move(handle)](bool success)
                        {
                            if(success)
                            {
                                dto::cis_cron_add_success res;

                                handle.send(res);
                            }
                            else
                            {
                                handle.send_error("Can't add cron.");
                            }
                        })
                .run();
        }

        return;
    }

    if(!permitted)
    {
        return request_context::authorized(ctx.client_info)
               ? tr.send_error(dto::user_permissions_error_access_denied{}, "Action not permitted.")
               : tr.send_error(dto::user_error_login_required{}, "Login required.");
    }

    dto::cis_job_error_doesnt_exist err;

    err.project = req.project;
    err.job = req.job;

    return handle.send_error(err, "Job doesn't exists.");
}

} // namespace handlers

} // namespace websocket
