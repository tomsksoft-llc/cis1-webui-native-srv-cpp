#pragma once

namespace meta
{

template<class... Ts>
struct overloaded
    : public Ts...
{
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace meta
