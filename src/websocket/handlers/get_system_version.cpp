#include "websocket/handlers/get_system_version.h"

#include "websocket/dto/system_version_info_success.h"

namespace websocket
{

namespace handlers
{

void get_system_version(
        configuration_manager& config,
        request_context& ctx,
        const dto::system_version_info& req,
        cis1::proto_utils::transaction tr)
{
    dto::system_version_info_success res{};
  
    res.webui_version.append(*config.get_entry<std::string>("cis/version"));

    if(config.has_entry("git/sha1") && config.has_entry("git/dirty"))
    {
        res.webui_version.append(" ");
        res.webui_version.append(*config.get_entry<std::string>("git/sha1"));
        res.webui_version.append(
                *config.get_entry<bool>("git/dirty")
                ? " dirty"
                : "");
    }

    tr.send(res);
}

} // namespace handlers

} // namespace websocket
