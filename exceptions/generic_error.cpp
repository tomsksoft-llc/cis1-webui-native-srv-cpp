#include "generic_error.h"

generic_error::generic_error(const std::string& what_arg)
    : base_cis_srv_exception(what_arg)
{}

generic_error::generic_error(const char* what_arg)
    : base_cis_srv_exception(what_arg)
{}
