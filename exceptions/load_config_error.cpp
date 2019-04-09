#include "load_config_error.h"

load_config_error::load_config_error(const std::string& what_arg)
    : base_cis_srv_exception(what_arg)
{}

load_config_error::load_config_error(const char* what_arg)
    : base_cis_srv_exception(what_arg)
{}
