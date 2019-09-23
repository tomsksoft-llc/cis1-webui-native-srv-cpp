#pragma once

namespace meta
{

template <class... Formats, size_t... Is>
std::tuple<Formats...> as_tuple(const std::smatch& arr, size_t offset,
                                std::index_sequence<Is...>)
{
    return std::make_tuple(boost::lexical_cast<Formats>(arr[Is + offset])...);
}

template <class ...Formats>
std::tuple<Formats...> as_tuple(const std::smatch& arr, size_t offset)
{
    return as_tuple<Formats...>(arr, offset, std::make_index_sequence<sizeof... (Formats)>{});
}

template <class ...Formats>
std::optional<std::tuple<Formats...>> maybe_tuple(const std::smatch& arr, size_t offset)
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

} // namespace meta
