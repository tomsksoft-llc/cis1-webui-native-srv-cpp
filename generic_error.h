#pragma once

#include "base_cis_srv_exception.h"

class generic_error
    : public base_cis_srv_exception
{
public:
    generic_error(const std::string& what_arg);
    generic_error(const char* what_arg);
    virtual ~generic_error() = default;
};
