#pragma once

namespace cis
{

void set_root_dir(const char* path);
const char* get_root_dir();

const char* const CORE = "/core";
const char* const PROJECTS = "/jobs";
const char* const SESSIONS = "/sessions";
const char* const LOGS = "/logs";

} // namespace cis
