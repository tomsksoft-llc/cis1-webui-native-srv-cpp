/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <filesystem>

#include "request_context.h"
#include "rights_manager.h"

bool validate_path(const std::filesystem::path& path);

std::optional<database::project_user_right> get_path_rights(
        request_context& ctx,
        rights_manager_interface& rights,
        const std::filesystem::path& path);
