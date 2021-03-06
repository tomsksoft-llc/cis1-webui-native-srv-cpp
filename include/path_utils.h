/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <filesystem>
#include <string>

#include "request_context.h"
#include "rights_manager.h"

bool validate_path(const std::filesystem::path& path);

bool validate_path_fragment(const std::string& fragment);

std::optional<project_rights> get_path_rights(
        const std::string& email,
        rights_manager_interface& rights,
        const std::filesystem::path& path,
        std::error_code& ec);
