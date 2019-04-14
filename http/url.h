#pragma once

#include <string>

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

namespace url
{

template<char ... Chars>
struct str
{
    static constexpr const char value[sizeof...(Chars)+1] = {Chars...,'\0'};
    static constexpr int size = sizeof...(Chars);
};

template<char ... Chars>
constexpr const char str<Chars...>::value[sizeof...(Chars)+1];

/* literal string operator template is GNU extension, will be in C++20
template<typename CharT, CharT ...String>
constexpr str<String...> operator"" _url()
{
    return str<String...>();
}
*/

template<typename  LambdaStrType>
struct string_builder
{
    template<unsigned... Indices>
    struct produce
    {
        typedef  str<LambdaStrType{}.chars[Indices]...>  result;
    };
};

#define URL_STR(VAR)\
    []() -> decltype(auto) {\
        struct  constexpr_string_type { const char * chars = VAR; };\
        return  ::meta::apply_range<sizeof(VAR)-1,\
            ::url::string_builder<constexpr_string_type>::produce>::result{};\
    }()

template <class ...>
struct concat_impl {};

template <char ...Str1, char ...Str2>
struct concat_impl<str<Str1...>, str<Str2...>>
{
    using value = str<Str1..., Str2...>;
};

template <class Str1, class Str2>
struct concat_str
{
    using value = typename concat_impl<Str1, Str2>::value;
};

template <class...>
struct str_cat {};

template <char... Str1, class... Strings>
struct str_cat<str<Str1...>, Strings...>
{
    using value = typename concat_str<str<Str1...>,
                             typename str_cat<Strings...>::value>::value;
};

template <>
struct str_cat<>
{
    using value = str<>;
};

template <class T>
struct token{};

template <>
struct token<std::string>
{
    using value_type = std::string;
    using regex = str<'(', '.', '+', ')'>;
};

template <>
struct token<int>
{
    using value_type = int;
    using regex = str<'(','\\','d','+',')'>;
};

struct ignore
{
    using regex = str<'.','+'>;
};

using string = token<std::string>;

using integer = token<int>;

template <class ParseString, class... Args>
struct url_chain
{
    template <char... Args2>
    constexpr auto operator<<(const str<Args2...>& t)
    {
        using result_str = typename concat_str<
            ParseString,
            str<Args2...>>::value;
        return url_chain<result_str, Args...>();
    }

    template <class TokenType>
    constexpr auto operator<<(const token<TokenType>& t)
    {
        using result_str = typename concat_str<
            ParseString,
            typename token<TokenType>::regex>::value;
        return url_chain<result_str, Args..., typename token<TokenType>::value_type>();
    }
    constexpr auto operator<<(const ignore& t)
    {
        using result_str = typename concat_str<
            ParseString,
            typename ignore::regex>::value;
        return url_chain<result_str, Args...>();
    }
    template <char... Args2>
    constexpr auto operator/(const str<Args2...>& t)
    {
        using result_str = typename str_cat<
            ParseString,
            str<'/'>,
            str<Args2...>>::value;
        return url_chain<result_str, Args...>();
    }
    template <class TokenType>
    constexpr auto operator/(const token<TokenType>& t)
    {
        using result_str = typename str_cat<
            ParseString,
            str<'/'>,
            typename token<TokenType>::regex>::value;
        return url_chain<result_str, Args..., typename token<TokenType>::value_type>();
    }
    constexpr auto operator/(const ignore& t)
    {
        using result_str = typename str_cat<
            ParseString,
            str<'/'>,
            typename ignore::regex>::value;
        return url_chain<result_str, Args...>();
    }
};

constexpr url_chain<str<>> make()
{
    return {};
}

constexpr url_chain<str<'/'>> root()
{
    return {};
}

} // url
