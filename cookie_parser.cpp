#include "cookie_parser.h"

#include "request_util.h"

handle_result cookie_parser::parse(
        http::request<http::string_body>& req,
        http_session::queue& /*queue*/,
        request_context& ctx)
{
    std::string cookies_string{};
    if(auto it = req.find("Cookie"); it != req.cend())
    {
        cookies_string = it->value();
    }
    ctx.cookies = parse_cookies(cookies_string);
    return handle_result::next;
}
