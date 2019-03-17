#pragma once

namespace cis
{

void set_root_dir(const char* path);
const char* get_root_dir();

static constexpr const char* const CORE = "/core";
static constexpr const char* const PROJECTS = "/jobs";
static constexpr const char* const SESSIONS = "/sessions";
static constexpr const char* const LOGS = "/logs";

} // namespace cis

namespace db
{

void set_root_dir(const char* path);
const char* get_root_dir();

} // namespace db
