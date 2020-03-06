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

std::optional<database::project_user_right> get_path_rights(
        const std::string& email,
        rights_manager_interface& rights,
        const std::filesystem::path& path,
        std::error_code& ec)
{
    if(path.begin() != path.end())
    {
        auto path_it = path.begin();

        ++path_it;

        if(path_it != path.end())
        {
            return rights.check_project_right(
                    email,
                    path_it->string(),
                    ec);
        }
    }

    return std::nullopt;
}
