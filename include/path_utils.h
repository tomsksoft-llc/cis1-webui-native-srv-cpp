#pragma once

#include <filesystem>

#include "request_context.h"
#include "rights_manager.h"

bool validate_path(const std::filesystem::path& path);

std::optional<database::project_user_right> get_path_rights(
        request_context& ctx,
        rights_manager& rights,
        const std::filesystem::path& path);
