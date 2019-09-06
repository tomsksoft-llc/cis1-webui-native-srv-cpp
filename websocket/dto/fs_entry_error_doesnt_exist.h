#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct fs_entry_error_doesnt_exist
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_error_doesnt_exist>()
                .done();
    }
};

} // namespace dto

} // namespace websocket
