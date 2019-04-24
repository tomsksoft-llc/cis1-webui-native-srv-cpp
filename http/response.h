#pragma once

#include <boost/beast.hpp>

namespace beast = boost::beast;

namespace http
{

namespace response
{

using string_response_t =
    boost::beast::http::response<boost::beast::http::string_body>;
using string_request_t =
    boost::beast::http::request<boost::beast::http::empty_body>;

string_response_t accepted(string_request_t&& req);

string_response_t not_found(string_request_t&& req);

string_response_t bad_request(
            string_request_t&& req,
            boost::beast::string_view why);

string_response_t server_error(
            string_request_t&& req,
            boost::beast::string_view what);

} // namespace response

} // namespace http
