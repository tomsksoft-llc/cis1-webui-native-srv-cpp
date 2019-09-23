#include "response.h"

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
