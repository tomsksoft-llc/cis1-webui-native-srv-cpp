#pragma once

#include "base_cis_srv_exception.h"

class load_config_error
    : public base_cis_srv_exception
{
public:
    load_config_error(const std::string& what_arg);
    load_config_error(const char* what_arg);
    virtual ~load_config_error() = default;
};
