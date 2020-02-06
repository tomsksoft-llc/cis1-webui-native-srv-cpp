/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/add_cis_job.h"

#include "websocket/dto/cis_job_add_success.h"
#include "websocket/dto/cis_job_error_already_exist.h"
#include "websocket/dto/cis_project_error_doesnt_exist.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void add_cis_job(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_job_add& req,
        cis1::proto_utils::transaction tr)
{
    auto project = cis_manager.get_project_info(req.project);

    std::error_code ec;

    auto perm = rights.check_project_right(ctx.client_info, req.project, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    auto permitted = perm && perm.value().write;

    if(project != nullptr && permitted)
    {
        auto job_path =
                std::filesystem::path{"/"} / req.project / req.job;

        auto& fs = cis_manager.fs();

        if(auto it = fs.find(job_path); it != fs.end())
        {
            dto::cis_job_error_already_exist err;
            err.project = req.project;
            err.job = req.job;

            return tr.send_error(err, "Job already exits.");
        }

        std::error_code ec;

        fs.create_directory(
                job_path,
                ec);

        if(ec)
        {
            return tr.send_error("Can't create directory.");
        }

        auto job_script = fs.create_file_w(
                job_path / "script",
                ec);
        if(ec)
        {
            return tr.send_error("Can't create file.");
        }

        fs.set_permissions(
                job_path / "script",
                std::filesystem::perms::owner_all
                    | std::filesystem::perms::group_read
                    | std::filesystem::perms::others_read,
                ec);
        if(ec)
        {
            return tr.send_error("Can't make script executable.");
        }

        auto job_params = fs.create_file_w(
                job_path / "job.params",
                ec);
        if(ec)
        {
            return tr.send_error("Can't create file.");
        }

        auto job_conf = fs.create_file_w(
                job_path / "job.conf",
                ec);
        if(ec)
        {
            return tr.send_error("Can't create file.");
        }

        *job_conf << "script=script" << '\n';
        *job_conf << "keep_last_success_builds=5" << '\n';
        *job_conf << "keep_last_break_builds=5";

        dto::cis_job_add_success res;

        return tr.send(res);
    }

    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    dto::cis_project_error_doesnt_exist err;
    err.project = req.project;

    return tr.send_error(err, "Job doesn't exists.");
}

} // namespace handlers

} // namespace websocket
