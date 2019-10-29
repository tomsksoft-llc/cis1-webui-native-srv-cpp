#include "websocket/handlers/get_project_info.h"

#include "websocket/dto/cis_project_info_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/cis_project_error_doesnt_exist.h"

#include "websocket/handlers/utils/make_dir_entry.h"

namespace websocket
{

namespace handlers
{

void get_project_info(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_project_info& req,
        cis1::proto_utils::transaction tr)
{
    auto project = cis_manager.get_project_info(req.project);

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(project != nullptr && permitted)
    {
        dto::cis_project_info_success res;

        for(auto& entry : project->get_job_list())
        {
            auto res_entry = std::visit(
                    meta::overloaded{
                        [&](const std::shared_ptr<fs_entry_interface>& entry)
                        {
                                return make_dir_entry(
                                        cis_manager.fs().root().path(),
                                        *entry);
                        },
                        [&](const std::shared_ptr<job_interface>& job)
                        {
                            auto entry = make_dir_entry(
                                    cis_manager.fs().root().path(),
                                    *job);

                            entry.metainfo = dto::fs_entry::job_info{};

                            return entry;
                        }
                    },
                    entry);

            res.fs_entries.push_back(res_entry);
        }

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
