/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

namespace cis
{

void set_root_dir(const char* path);
const char* get_root_dir();

static constexpr const char* const core = "core";
static constexpr const char* const projects = "jobs";
static constexpr const char* const sessions = "sessions";
static constexpr const char* const logs = "logs";

} // namespace cis
