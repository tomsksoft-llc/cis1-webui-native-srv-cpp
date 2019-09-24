#include "websocket/handlers/remove_api_key.h"

#include "websocket/dto/user_api_key_remove_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void remove_api_key(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::user_api_key_remove& req,
        transaction tr)
{
    if(ctx.username == req.username
        || (!ctx.username.empty()
        && authentication_handler.get_group(ctx.username).value() == "admin"))
    {
        auto result = authentication_handler.remove_api_key(req.username);

        if(!result)
        {
            return tr.send_error("Can't remove APIAccessSecretKey.");
        }

        dto::user_api_key_remove_success res;

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted.");
}

} // namespace handlers

} // namespace websocket
