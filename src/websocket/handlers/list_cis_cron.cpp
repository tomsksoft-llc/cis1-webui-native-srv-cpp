#include "websocket/handlers/list_cis_cron.h"

#include "websocket/dto/cis_cron_list_success.h"

namespace websocket
{

namespace handlers
{

void list_cis_cron(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_cron_list& req,
        cis1::proto_utils::transaction tr)
{
    if(auto executor = tr.get_executor(); executor)
    {
        make_async_chain(executor.value())
            .then(cis_manager.list_cron(req.mask))
            .then([tr]( bool success,
                        const std::vector<cis::cron_entry>& entries)
                    {
                        if(success)
                        {
                            dto::cis_cron_list_success res;

                            for(auto& entry : entries)
                            {
                                res.entries.push_back({
                                        entry.project,
                                        entry.job,
                                        entry.cron_expr});
                            }

                            tr.send(res);
                        }
                        else
                        {
                            tr.send_error("Cron list error.");
                        }
                    })
            .run();
    }
}

} // namespace handlers

} // namespace websocket
