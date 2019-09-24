#pragma once

namespace meta
{

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

} // namespace meta
