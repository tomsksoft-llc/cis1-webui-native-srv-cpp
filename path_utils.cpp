#include "path_utils.h"

bool validate_path(const std::filesystem::path& path)
{
    return path.root_path() == "/";
}

std::optional<database::project_user_right> get_path_rights(
        request_context& ctx,
        rights_manager& rights,
        const std::filesystem::path& path)
{
    if(path.begin() != path.end())
    {
        auto path_it = path.begin();
        ++path_it;
        if(path_it != path.end())
        {
            return rights.check_project_right(
                    ctx.username,
                    path_it->string());
        }
    }

    return std::nullopt;
}
