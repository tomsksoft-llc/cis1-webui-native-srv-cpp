/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "http/response.h"

#include <string>

namespace http
{

namespace response
{

beast::http::response<beast::http::string_body> accepted(
        beast::http::request<beast::http::empty_body>&& req)
{
    beast::http::response<beast::http::string_body> res{
            beast::http::status::accepted,
            req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "Request accepted.";
    res.prepare_payload();

    return res;
}

beast::http::response<beast::http::string_body> method_not_allowed(
        beast::http::request<beast::http::empty_body>&& req,
        const std::vector<beast::http::verb>& allowed_verbs)
{
    beast::http::response<beast::http::string_body> res{
            beast::http::status::method_not_allowed,
            req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");
    
    std::stringstream allow_string;
    
    for(auto it = allowed_verbs.begin(); it != allowed_verbs.end(); ++it)
    {
        if(it != allowed_verbs.begin())
        {
            allow_string << ", ";
        }

        allow_string << *it;
    }

    res.set(beast::http::field::allow, allow_string.str());
    res.keep_alive(req.keep_alive());
    res.body() = "Method not allowed.";
    res.prepare_payload();

    return res;
}

beast::http::response<beast::http::string_body> not_found(
        beast::http::request<beast::http::empty_body>&& req)
{
    beast::http::response<beast::http::string_body> res{
            beast::http::status::not_found,
                req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '"
            + std::string(req.target())
            + "' was not found.";
    res.prepare_payload();

    return res;
}

beast::http::response<beast::http::string_body> forbidden(
        beast::http::request<beast::http::empty_body>&& req)
{
    beast::http::response<beast::http::string_body> res{
            beast::http::status::forbidden,
                req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "Forbidden.";
    res.prepare_payload();

    return res;
}

beast::http::response<beast::http::string_body> bad_request(
        beast::http::request<beast::http::empty_body>&& req,
        beast::string_view why)
{
    beast::http::response<beast::http::string_body> res{
            beast::http::status::bad_request,
            req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();

    return res;
}

beast::http::response<beast::http::string_body> server_error(
        beast::http::request<beast::http::empty_body>&& req,
        beast::string_view what)
{
    beast::http::response<beast::http::string_body> res{
            beast::http::status::internal_server_error,
            req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();

    return res;
}

} // namespace response

} // namespace http
