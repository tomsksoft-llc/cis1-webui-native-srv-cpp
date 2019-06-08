#pragma once

#include <string>
#include <regex>

#include <boost/lexical_cast.hpp>

#include "meta.h"

namespace url
{

template <class T>
struct token{};

template <>
struct token<std::string>
{
    using value_type = std::string;
    using regex = meta::ct_string<'(', '.', '+', ')'>;
};

struct bound_string_tag;

template <>
struct token<bound_string_tag>
{
    using value_type = std::string;
    using regex = meta::ct_string<'(', '[','^','?','/',']', '+', ')'>;
};

struct query_string_tag;

template <>
struct token<query_string_tag>
{
    using value_type = std::string;
    using regex = meta::ct_string<
            '(', '\\', '?', '(', '?', ':', '[', '^', '=', ']', '+', ')', '\\',
            '=', '(', '?', ':', '[', '^', '&', ']', '+', ')', '(', '?',
            ':', '\\', '&', '(', '?', ':', '[', '^', '=', ']', '+', ')',
            '\\', '=', '(', '?', ':', '[', '^', '&', ']', '+', ')', ')', '*', ')', '?'>;
};

template <>
struct token<int>
{
    using value_type = int;
    using regex = meta::ct_string<'(','\\','d','+',')'>;
};

struct ignore
{
    using regex = meta::ct_string<'.','+'>;
};

using string = token<std::string>;

using bound_string = token<bound_string_tag>;

using query_string = token<query_string_tag>;

using integer = token<int>;

template <class ParseString, class... Args>
struct url_chain
{
    template <char... Args2>
    constexpr auto operator<<(const meta::ct_string<Args2...>& t)
    {
        using result_str = typename meta::concat_string<
                ParseString,
                meta::ct_string<Args2...>>::value;
        return url_chain<result_str, Args...>();
    }

    template <class TokenType>
    constexpr auto operator<<(const token<TokenType>& t)
    {
        using result_str = typename meta::concat_string<
                ParseString,
                typename token<TokenType>::regex>::value;
        return url_chain<result_str, Args..., typename token<TokenType>::value_type>();
    }
    constexpr auto operator<<(const ignore& t)
    {
        using result_str = typename meta::concat_string<
                ParseString,
                typename ignore::regex>::value;
        return url_chain<result_str, Args...>();
    }
    template <char... Args2>
    constexpr auto operator/(const meta::ct_string<Args2...>& t)
    {
        using result_str = typename meta::string_cat<
                ParseString,
                meta::ct_string<'/'>,
                meta::ct_string<Args2...>>::value;
        return url_chain<result_str, Args...>();
    }
    template <class TokenType>
    constexpr auto operator/(const token<TokenType>& t)
    {
        using result_str = typename meta::string_cat<
                ParseString,
                meta::ct_string<'/'>,
                typename token<TokenType>::regex>::value;
        return url_chain<result_str, Args..., typename token<TokenType>::value_type>();
    }
    constexpr auto operator/(const ignore& t)
    {
        using result_str = typename meta::string_cat<
                ParseString,
                meta::ct_string<'/'>,
                typename ignore::regex>::value;
        return url_chain<result_str, Args...>();
    }
};

constexpr url_chain<meta::ct_string<>> make()
{
    return {};
}

constexpr url_chain<meta::ct_string<'/'>> root()
{
    return {};
}

} // url
