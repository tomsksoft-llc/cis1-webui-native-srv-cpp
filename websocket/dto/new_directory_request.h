#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct new_directory_request
{
    std::string path;
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<new_directory_request>()
                .add_field(
                        CT_STRING("path"),
                        ptr_v<&new_directory_request::path>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
