#include "request_util.h"

#include <boost/regex.hpp>

namespace http
{

const boost::regex query_begin_regex("^\\??([^=]+)\\=([^&]+)");
const boost::regex query_entry_regex("\\&([^=]+)\\=([^&]+)");
const boost::regex cookies_regex("(.*?)=(.*?)($|;|,(?! ))");

unsigned char to_hex(unsigned char ch)
{
    return ch + (ch > 9 ? ('A' - 10) : '0');
}

unsigned char from_hex(unsigned char ch)
{
    if(ch >= '0' && ch <= '9')
    {
        ch -= '0';
    }
    else if(ch >= 'a' && ch <= 'f')
    {
        ch -= 'a' - 10;
    }
    else if(ch >= 'A' && ch <= 'F')
    {
        ch -= 'A' - 10;
    }
    else
    {
        ch = 0;
    }
    return ch;
}

std::string unescape_uri(const std::string& escaped_uri)
{
    std::string result(escaped_uri.size(), '\0');
    size_t j = 0;
    for(size_t i = 0; i < escaped_uri.size(); ++i, ++j)
    {
        if(escaped_uri[i] == '+')
        {
            result[j] = ' ';
        }
        else if(escaped_uri[i] == '%' && escaped_uri.size() > i + 2)
        {
            result[j] =
                    from_hex(escaped_uri[i+1]) * 16
                    + from_hex(escaped_uri[i+2]);
            i += 2;
        }
        else
        {
            result[j] = escaped_uri[i];
        }
    }
    result.resize(j);
    return result;
}

std::string escape_uri(const std::string& unescaped_uri)
{
    std::string result;
    const auto size_factor = 1.33;
    result.reserve(unescaped_uri.size() * size_factor);
    for(auto ch : unescaped_uri)
    {
        if(    (ch >= 'a' && ch <= 'z')
            || (ch >= 'A' && ch <= 'Z')
            || (ch >= '0' && ch <= '9'))
        { // allowed
            result += ch;
        }
        else if(ch == ' ')
        {
            result += '+';
        }
        else
        {
            result += '%';
            result += to_hex(ch * 16);
            result += to_hex(ch % 16);
        }
    }
    return result;
}

std::map<std::string, std::string> parse_request_query(const std::string& body)
{
    std::map<std::string, std::string> result;

    auto start = body.cbegin();
    auto end = body.cend();
    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;

    regex_search(start, end, what, query_begin_regex, flags);

    result.insert({what[1], what[2]});
    start = what[0].second;
    // update flags:
    flags |= boost::match_prev_avail;
    flags |= boost::match_not_bob;

    while(regex_search(start, end, what, query_entry_regex, flags))
    {
        result.insert({what[1], what[2]});

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
