/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <system_error>
#include <string>

namespace cis
{

enum class error_code
{
    ok,
    cant_parse_config_ini,
    incorrect_environment,
    cant_run_http_listener,
    cant_resolve_address,
    cant_open_file,
    cant_cast_config_entry,
    database_error,
};

std::error_code make_error_code(error_code ec);

struct error_category
        : public std::error_category
{
    const char * name() const noexcept override;
    std::string message(int ev) const override;
};

} // namespace cis

namespace std
{

template<>
struct is_error_code_enum<cis::error_code>
    : public true_type
{};

} // namespace std
