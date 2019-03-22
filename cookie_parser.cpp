#include "cookie_parser.h"

#include "request_util.h"

web_app::handle_result cookie_parser::parse(
        web_app::request_t& req,
        web_app::queue_t& /*queue*/,
        request_context& ctx)
{
    std::string cookies_string{};
    if(auto it = req.find("Cookie"); it != req.cend())
    {
        cookies_string = it->value();
    }
    ctx.cookies = parse_cookies(cookies_string);
    return web_app::handle_result::next;
}
