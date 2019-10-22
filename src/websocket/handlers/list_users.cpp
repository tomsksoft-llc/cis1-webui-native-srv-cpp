#include "websocket/handlers/list_users.h"

#include "websocket/dto/user_list_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void list_users(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_list& req,
        cis1::proto_utils::transaction tr)
{
    auto perm = rights.check_user_permission(ctx.username, "users.list");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        const auto users = authentication_handler.get_user_infos();

        dto::user_list_success res;

        for(auto& user : users)
        {
            res.users.push_back({
                    user.name,
                    user.email,
                    user.group,
                    false,
                    user.api_access_key ? user.api_access_key.value() : ""});
        }

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted");
}

} // namespace handlers

} // namespace websocket
