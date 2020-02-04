/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/list_projects.h"

#include "websocket/dto/cis_project_list_get_success.h"

#include "websocket/handlers/utils/make_dir_entry.h"

#include "cis/dirs.h"

namespace websocket
{

namespace handlers
{

void list_projects(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_project_list_get& req,
        cis1::proto_utils::transaction tr)
{
    dto::cis_project_list_get_success res;

    for(auto& entry : cis_manager.get_project_list())
    {
        auto res_entry = std::visit(
                meta::overloaded{
                    [&](const std::shared_ptr<fs_entry_interface>& entry)
                    {
                            return make_dir_entry(
                                    cis_manager.fs().root().path(),
                                    *entry);
                    },
                    [&](const std::shared_ptr<project_interface>& project)
                    {
                        auto entry = make_dir_entry(
                                cis_manager.fs().root().path(),
                                *project);

                        bool permitted = false;

                        std::error_code ec;

                        auto perm = rights.check_project_right(ctx.cln_info, entry.name, ec);

                        //ignore ec

                        if((perm.has_value() && perm.value().read) || !perm.has_value())
                        {
                            permitted = true;
                        }

                        entry.metainfo = dto::fs_entry::project_info{permitted};

                        return entry;
                    }
                },
                entry);

        res.fs_entries.push_back(res_entry);
    }

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
