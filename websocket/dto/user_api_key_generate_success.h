#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct user_api_key_generate_success
{
    std::string api_key;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_api_key_generate_success>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("api_key"),
                        CT_STRING("generate"),
                        CT_STRING("success"))
                .add_field(
                        CT_STRING("APIAccessSecretKey"),
                        ptr_v<&user_api_key_generate_success::api_key>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
