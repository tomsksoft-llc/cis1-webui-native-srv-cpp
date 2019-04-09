#include "cookie_parser.h"

#include "request_util.h"

handle_result cookie_parser::parse(
        http::request<http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& /*queue*/)
{
    std::string cookies_string{};
    if(auto it = req.find("Cookie"); it != req.cend())
    {
        cookies_string = it->value();
    }
    ctx.cookies = parse_cookies(cookies_string);
    return handle_result::next;
}
