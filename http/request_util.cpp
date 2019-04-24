#include "request_util.h"

#include <boost/regex.hpp>

namespace http
{

const boost::regex query_regex("(^|&)([a-zA-Z0-9]+)\\=([a-zA-Z0-9]+)");
const boost::regex cookies_regex("(.*?)=(.*?)($|;|,(?! ))");

std::map<std::string, std::string> parse_request_query(const std::string& body)
{
    std::map<std::string, std::string> result;

    auto start = body.cbegin();
    auto end = body.cend();
    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;

    while(regex_search(start, end, what, query_regex, flags))
    {
        if(what.size() < 4)
        {
            continue;
        }
        result.insert({what[2], what[3]});

        start = what[0].second;
        // update flags:
        flags |= boost::match_prev_avail;
        flags |= boost::match_not_bob;
    }

    return result;
}

std::map<std::string, std::string> parse_cookies(const std::string& cookies)
{
    std::map<std::string, std::string> result;

    auto start = cookies.cbegin();
    auto end = cookies.cend();
    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;

    while(regex_search(start, end, what, cookies_regex, flags))
    {
        if(what.size() < 3)
        {
            continue;
        }
        result.insert({what[1], what[2]});

        start = what[0].second;
        // update flags:
        flags |= boost::match_prev_avail;
        flags |= boost::match_not_bob;
    }

    return result;
}

} // namespace http
