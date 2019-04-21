#pragma once

#include <utility>
#include <tuple>

#include <boost/regex.hpp>

namespace meta
{
    
template <class... Formats, size_t... Is>
std::tuple<Formats...> as_tuple(const boost::smatch& arr, size_t offset,
                                std::index_sequence<Is...>)
{
    return std::make_tuple(boost::lexical_cast<Formats>(arr[Is + offset])...);
}

template <class ...Formats>
std::tuple<Formats...> as_tuple(const boost::smatch& arr, size_t offset)
{
    return as_tuple<Formats...>(arr, offset, std::make_index_sequence<sizeof... (Formats)>{});
}

template <class ...Formats>
std::optional<std::tuple<Formats...>> maybe_tuple(const boost::smatch& arr, size_t offset)
{
    std::tuple<Formats...> result;
    try
    {
        result = as_tuple<Formats...>(arr, offset);
    }
    catch(...)
    {
        return std::nullopt;
    }

    return result;
}

template<
    unsigned Count,
    template<unsigned...> class MetaFunctor,
    unsigned... Indices>
struct apply_range
{
    using result = typename apply_range<
        Count - 1,
        MetaFunctor,
        Count - 1,
        Indices...>::result;
};

template<
    template<unsigned...> class MetaFunctor,
    unsigned... Indices>
struct apply_range<0, MetaFunctor, Indices...>
{
    using result = typename MetaFunctor<Indices...>::result;
};

template<char ... Chars>
struct ct_string
{
    static constexpr const char value[sizeof...(Chars)+1] = {Chars...,'\0'};
    static constexpr int size = sizeof...(Chars);
};

template<char ... Chars>
constexpr const char ct_string<Chars...>::value[sizeof...(Chars)+1];

/* literal string operator template is GNU extension, will be in C++20 standard
template<typename CharT, CharT ...String>
constexpr ct_string<String...> operator"" _url()
{
    return ct_string<String...>();
}
*/

template<typename  LambdaStrType>
struct ct_string_builder
{
    template<unsigned... Indices>
    struct produce
    {
        typedef  ct_string<LambdaStrType{}.chars[Indices]...>  result;
    };
};

#define CT_STRING(VAR)\
    []() -> decltype(auto) {\
        struct  constexpr_string_type { const char * chars = VAR; };\
        return  ::meta::apply_range<sizeof(VAR)-1,\
            ::meta::ct_string_builder<constexpr_string_type>::produce>::result{};\
    }()

template <class ...>
struct concat_impl {};

template <char ...Str1, char ...Str2>
struct concat_impl<ct_string<Str1...>, ct_string<Str2...>>
{
    using value = ct_string<Str1..., Str2...>;
};

template <class Str1, class Str2>
struct concat_string
{
    using value = typename concat_impl<Str1, Str2>::value;
};

template <class...>
struct string_cat {};

template <char... Str1, class... Strings>
struct string_cat<ct_string<Str1...>, Strings...>
{
    using value = typename concat_string<ct_string<Str1...>,
                             typename string_cat<Strings...>::value>::value;
};

template <>
struct string_cat<>
{
    using value = ct_string<>;
};

} // namespace meta
