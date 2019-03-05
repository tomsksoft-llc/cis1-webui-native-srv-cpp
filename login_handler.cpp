#include "login_handler.h"

#include "http_session.h"
#include "router.h"
#include "request_util.h"

using namespace std::string_literals;

login_handler::login_handler(
            std::function<std::string(const std::string&, const std::string&)> authenticate_fn,
            std::function<bool(const std::string&)> authorize_fn)
    : authenticate_(authenticate_fn)
    , authorize_(authorize_fn)
{}

void login_handler::handle(
            http::request<http::string_body>&& req,
            http_session::queue& queue)
{
    if(req.method() == http::verb::post)
    {
        http::response<http::empty_body> res{http::status::found, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        
        auto parsed = parse_request_query(req.body());
        
        bool login_failed = false;
        if(!parsed.count("uname") || !parsed.count("pass"))
        {
            res.set(http::field::location, "/login_failed.html");
            login_failed = true;
        }
        
        auto token = authenticate_(parsed["uname"], parsed["pass"]);

        if(token.empty())
        {
            res.set(http::field::location, "/login_failed.html");
            login_failed = true;
        }
        
        if(!login_failed)
        {
            auto origin = req.find("origin");
            if(origin != req.end())
            {
                res.set(http::field::location, origin->value());
            }
            else
            {
                res.set(http::field::location, "/");
            }
            res.insert(http::field::set_cookie, "AuthToken="s + token);
        }
        res.keep_alive(req.keep_alive());
        return queue.send(std::move(res));
    }
    else
    {
        return queue.send(handlers::not_found(std::move(req)));
    }
}

bool login_handler::authorize(http::request<http::string_body>& req)
{
    auto cookies_it = req.find("Cookie");
    if(cookies_it != req.cend())
    {
        auto cookies = parse_cookies(cookies_it->value().to_string());
        if(cookies.count("AuthToken"))
        {
            return authorize_(cookies["AuthToken"]);
        }
    }
    return false;
}
