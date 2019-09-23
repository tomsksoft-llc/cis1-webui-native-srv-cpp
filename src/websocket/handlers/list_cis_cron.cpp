#include "list_cis_cron.h"

#include "websocket/dto/cis_cron_list_success.h"

namespace websocket
{

namespace handlers
{

void list_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_cron_list& req,
        transaction tr)
{
    if(auto executor = tr.get_executor(); executor)
    {
        make_async_chain(executor.value())
            .then(cis_manager.list_cron(req.mask))
            .then([tr](const std::vector<cis::cron_entry>& entries)
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
                    })
            .run();
    }
}

} // namespace handlers

} // namespace websocket
