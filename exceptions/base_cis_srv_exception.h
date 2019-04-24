#pragma once

#include <exception>
#include <string>

class base_cis_srv_exception
    : public std::exception
{
    std::string what_;
public:
    base_cis_srv_exception(const std::string& what_arg);
    base_cis_srv_exception(const char* what_arg);
    virtual ~base_cis_srv_exception() = default;
    base_cis_srv_exception(const base_cis_srv_exception& other);
    base_cis_srv_exception& operator=(const base_cis_srv_exception& other);
    virtual const char* what() const noexcept;
};
