#include "websocket/handlers/session_unsubscribe.h"

namespace websocket
{

namespace handlers
{

void session_unsubscribe(
        cis::cis_manager_interface& cis_manager,
        request_context& ctx,
        const dto::cis_session_unsubscribe& req,
        cis1::proto_utils::transaction tr)
{
    auto s = cis_manager.get_session_subscriber(
            req.session_id,
            ctx.session_id);

    if(s)
    {
        s->unsubscribe();
    }
}

} // namespace handlers

} // namespace websocket
