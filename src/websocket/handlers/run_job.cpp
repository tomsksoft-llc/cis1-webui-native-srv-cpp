#include "websocket/handlers/run_job.h"

#include "websocket/dto/cis_job_run_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/cis_job_error_doesnt_exist.h"
#include "websocket/dto/cis_job_error_invalid_params.h"

namespace websocket
{

namespace handlers
{

void run_job(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_job_run& req,
        transaction tr)
{
    std::map<std::string, std::string> params;

    for(auto& param : req.params)
    {
        params.insert({param.name, param.value});
    }

    auto* job = cis_manager.get_job_info(req.project, req.job);

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().execute : true;

    if(job != nullptr && permitted)
    {
        auto& job_params = job->get_params();
        std::vector<std::string> param_values;
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
                param_values.push_back(param.default_value);
            }
            else
            {
                param_values.push_back(params[param.name]);
            }
        }

        cis_manager.run_job(
                req.project,
                req.job,
                param_values);

        dto::cis_job_run_success res;

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