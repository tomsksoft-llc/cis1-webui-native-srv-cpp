#include "base_cis_srv_exception.h"

base_cis_srv_exception::base_cis_srv_exception(std::string what_arg)
    : what_(std::move(what_arg))
{}

base_cis_srv_exception::base_cis_srv_exception(const char* what_arg)
    : what_(what_arg)
{}

base_cis_srv_exception::base_cis_srv_exception(const base_cis_srv_exception& other)
    : what_(other.what_)
{}

base_cis_srv_exception& base_cis_srv_exception::operator=(const base_cis_srv_exception& other)
{
    what_ = other.what_;
    return *this;
}

const char* base_cis_srv_exception::what() const noexcept
{
    return what_.c_str();
}
