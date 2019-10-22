#include "cis/dirs.h"

#include <string>

namespace cis
{

std::string root_dir;

void set_root_dir(const char* path)
{
    root_dir = path;
}

const char* get_root_dir()
{
    return root_dir.c_str();
}

} // namespace cis
