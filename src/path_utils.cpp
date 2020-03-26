/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "path_utils.h"

bool validate_path(const std::filesystem::path& path)
{
    return path.root_path() == "/";
}

bool validate_path_fragment(const std::string& fragment)
{
    return fragment.find("/") == std::string::npos;
}

std::optional<project_rights> get_path_rights(
        const std::string& email,
        rights_manager_interface& rights,
        const std::filesystem::path& path,
        std::error_code& ec)
{
    if(path.begin() == path.end())
    {
        return std::nullopt;
    }

    auto path_it = path.begin();

    ++path_it;

    if(path_it == path.end())
    {
        return std::nullopt;
    }

    // check if the user is admin because the requested entry may be root directory and not be a project
    const auto is_admin = rights.is_admin(email, ec);

    if(ec)
    {
        return std::nullopt;
    }

    if(is_admin)
    {
        return project_rights{true, true, true};
    }

    return rights.check_project_right(
            email,
            path_it->string(),
            ec);
}
