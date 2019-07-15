#pragma once

#define CT_STRING(VAR)\
    []() -> decltype(auto) {\
        struct  constexpr_string_type { const char * chars = VAR; };\
        return  ::meta::apply_range<sizeof(VAR)-1,\
            ::meta::ct_string_builder<constexpr_string_type>::produce>::result{};\
    }()
