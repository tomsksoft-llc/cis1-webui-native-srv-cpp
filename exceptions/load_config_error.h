#pragma once

#include "base_cis_srv_exception.h"

class load_config_error
    : public base_cis_srv_exception
{
public:
    using base_cis_srv_exception::base_cis_srv_exception;
    virtual ~load_config_error() = default;
};
