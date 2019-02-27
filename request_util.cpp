#include "request_util.h"

#include <boost/regex.hpp>

std::map<std::string, std::string> parse_request(const std::string& body)
{
    std::map<std::string, std::string> result;
    static boost::regex r("(^|&)([a-zA-Z0-9]+)\\=([a-zA-Z0-9]+)");
    auto start = body.cbegin();
    auto end = body.cend();
    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;
    while(regex_search(start, end, what, r, flags))
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

std::map<std::string, std::string> parse_cookies(const std::string& body)
{
    std::map<std::string, std::string> result;
    static boost::regex r("(.*?)=(.*?)($|;|,(?! ))");
    auto start = body.cbegin();
    auto end = body.cend();
    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;
    while(regex_search(start, end, what, r, flags))
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
