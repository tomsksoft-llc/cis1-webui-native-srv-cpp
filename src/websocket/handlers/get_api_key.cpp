#include "get_api_key.h"

#include "websocket/dto/user_api_key_get_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void get_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::user_api_key_get& req,
        transaction tr)
{

    if(ctx.username == req.username
        || (!ctx.username.empty()
        && authentication_handler.get_group(ctx.username).value() == "admin"))
    {
        auto api_key = authentication_handler.get_api_key(req.username);

        if(!api_key)
        {
            return tr.send_error("Can't retrieve APIAccessSecretKey.");
        }

        dto::user_api_key_get_success res;
        res.api_key = api_key.value();

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted.");
}

} // namespace handlers

} // namespace websocket
