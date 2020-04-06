/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/remove_cis_cron.h"

#include "websocket/dto/cis_job_error_doesnt_exist.h"
#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/cis_cron_remove_success.h"
#include "websocket/dto/auth_error_login_required.h"

#include "websocket/handlers/utils/check_ec.h"

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
    if(!ctx.client_info)
    {
        WSHU_LOG(scl::Level::Info, "Login required");

        return tr.send_error(dto::auth_error_login_required{}, "Login required.");
    }

    const auto& email = ctx.client_info.value().email;

    auto job = cis_manager.get_job_info(req.project, req.job);

    std::error_code ec;

    auto perm = rights.check_project_right(email, req.project, ec);

    WSHU_CHECK_EC(ec);

    auto permitted = perm && perm.value().write && perm.value().execute;

    if(job != nullptr && permitted)
    {
        if(auto executor = tr.get_executor(); executor)
        {
            make_async_chain(executor.value())
                .then(cis_manager.remove_cron(
                        req.project,
                        req.job,
                        req.cron_expr))
                .then([tr](bool success)
                        {
                            if(success)
                            {
                                dto::cis_cron_remove_success res;

                                tr.send(res);
                            }
                            else
                            {
                                LOG(scl::Level::Error, "Can't remove cron");

                                tr.send_error("Can't remove cron.");
                            }
                        })
                .run();
        }

        return;
    }

    if(!permitted)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    WSHU_LOG(scl::Level::Info, R"("%s/%s" job doesn't exist)", req.project, req.job);

    dto::cis_job_error_doesnt_exist err;

    err.project = req.project;
    err.job = req.job;

    return tr.send_error(err, "Job doesn't exist.");
}

} // namespace handlers

} // namespace websocket
