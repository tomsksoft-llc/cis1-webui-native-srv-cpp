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
#include "websocket/handlers/utils/wsh_log.h"

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

                        std::error_code ec;

                        bool permitted = false;
                        if(ctx.client_info)
                        {
                            const auto& email = ctx.client_info->email;
                            auto perm = rights.check_project_right(email, entry.name, ec);
                            //ignore ec
                            permitted = perm.has_value() && perm.value().read;
                        }

                        entry.metainfo = dto::fs_entry::project_info{permitted};

                        return entry;
                    }
                },
                entry);

        res.fs_entries.push_back(res_entry);
    }

    WSHU_LOG(scl::Level::Action, "Project list was sent");

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
