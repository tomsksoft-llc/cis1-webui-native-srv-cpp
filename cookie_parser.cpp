#include "cookie_parser.h"

#include "request_util.h"

web_app::handle_result cookie_parser::parse(
        web_app::request_t& req,
        web_app::queue_t& queue,
        web_app::context_t& ctx)
{
    std::string cookies_string{};
    if(auto it = req.find("Cookie"); it != req.cend())
    {
        cookies_string = it->value();
    }
    auto cookies = parse_cookies(cookies_string);
    ctx["cookies"] = cookies;
    return web_app::handle_result::next;
}
