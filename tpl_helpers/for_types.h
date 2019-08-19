#pragma once

#include "type_tag.h"

namespace meta
{

template <class... Types, class Fn>
void for_types(Fn&& cb)
{
    (cb(type_tag<Types>{}) && ...);
}

} // namespace meta
