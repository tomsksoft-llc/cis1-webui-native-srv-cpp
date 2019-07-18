#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct refresh_fs_entry_request
{
    std::string path;
    bool recursive;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<refresh_fs_entry_request>()
                .add_field(
                        CT_STRING("path"),
                        ptr_v<&refresh_fs_entry_request::path>{})
                .add_field(
                        CT_STRING("recursive"),
                        ptr_v<&refresh_fs_entry_request::recursive>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
