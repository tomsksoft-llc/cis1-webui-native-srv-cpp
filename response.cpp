#include "response.h"

namespace beast = boost::beast;
namespace http = beast::http;

http::response<http::string_body> response::accepted(
        http::request<http::string_body>&& req)
{
    http::response<http::string_body> res{http::status::accepted, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "Request accepted.";
    res.prepare_payload();
    return res;
}

http::response<http::string_body> response::not_found(
        http::request<http::string_body>&& req)
{
    http::response<http::string_body> res{http::status::not_found, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + std::string(req.target()) + "' was not found.";
    res.prepare_payload();
    return res;
}

http::response<http::string_body> response::bad_request(
        http::request<http::string_body>&& req, beast::string_view why)
{
    http::response<http::string_body> res{http::status::bad_request, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
}

http::response<http::string_body> response::server_error(
        http::request<http::string_body>&& req, beast::string_view what)
{
    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
}
