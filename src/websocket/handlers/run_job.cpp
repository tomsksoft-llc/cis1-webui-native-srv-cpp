/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/run_job.h"

#include "websocket/dto/cis_job_run_success.h"
#include "websocket/dto/cis_job_finished.h"
#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/cis_job_error_doesnt_exist.h"
#include "websocket/dto/cis_job_error_invalid_params.h"
#include "websocket/dto/user_error_login_required.h"

namespace websocket
{

namespace handlers
{

void run_job(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_job_run& req,
        cis1::proto_utils::transaction tr)
{
    if(!ctx.client_info)
    {
        return tr.send_error(dto::user_error_login_required{}, "Login required.");
    }

    const auto& email = ctx.client_info.value().email;

    std::map<std::string, std::string> params;

    for(auto& param : req.params)
    {
        params.insert({param.name, param.value});
    }

    auto job = cis_manager.get_job_info(req.project, req.job);

    std::error_code ec;

    auto perm = rights.check_project_right(email, req.project, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    auto permitted = perm && perm.value().execute;

    if(job != nullptr && permitted)
    {
        auto& job_params = job->get_params();
        std::vector<std::pair<std::string, std::string>> param_values;
        param_values.reserve(job_params.size());

        for(auto& param : job_params)
        {
            if(!params.count(param.name))
            {
                if(param.default_value.empty())
                {
                    dto::cis_job_error_invalid_params err;

                    return tr.send_error(err, "Invalid params.");
                }

                param_values.push_back({param.name, param.default_value});
            }
            else
            {
                param_values.push_back({param.name, params[param.name]});
            }
        }

        if(auto executor = tr.get_executor(); executor)
        {
            make_async_chain(executor.value())
                .then(cis_manager.run_job(
                        req.project,
                        req.job,
                        req.force,
                        param_values,
                        [tr](const std::string& session_id)
                        {
                            dto::cis_job_run_success res;
                            res.session_id = session_id;

                            tr.send(res);
                        },
                        [](const std::string& session_id){},
                        email))
                .then(  [tr](const cis::execution_info& info)
                        {
                            if(info.success && info.exit_code)
                            {
                                dto::cis_job_finished res;
                                res.status =
                                        info.exit_code.value() == 0
                                        ? "success"
                                        : "failed";
                                res.exit_code = info.exit_code.value();
                                res.exit_message = info.exit_message;
                                res.session_id = info.session_id;

                                tr.send(res);
                            }
                        })
                .run();
        }

        return;
    }

    if(!permitted)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    dto::cis_job_error_doesnt_exist err;
    err.project = req.project;
    err.job = req.job;

    return tr.send_error(err, "Job doesn't exists.");
}

} // namespace handlers

} // namespace websocket
