#pragma once

#include <utility>
#include <tuple>
#include <regex>

#include "apply_range.h"

namespace meta
{

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
        using result = ct_string<LambdaStrType{}.chars[Indices]...>;
    };
};

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
    using value = typename concat_string<
            ct_string<Str1...>,
            typename string_cat<Strings...>::value>::value;
};

template <>
struct string_cat<>
{
    using value = ct_string<>;
};

namespace string_detail
{
	template <typename S, std::size_t ...N>
	constexpr meta::ct_string<S::get()[N]...>
		prepare_impl(S, std::index_sequence<N...>)
	{
		return {};
	}

	template <typename S>
	constexpr decltype(auto) prepare(S s) {
		return prepare_impl(s,
			std::make_index_sequence<sizeof(S::get()) - 1>{});
	}
}

} // namespace meta
