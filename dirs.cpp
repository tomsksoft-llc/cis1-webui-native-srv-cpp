#include "dirs.h"

namespace cis
{

const char* root_dir = nullptr;

void set_root_dir(const char* path)
{
    root_dir = path;
}

const char* get_root_dir()
{
    return root_dir;
}

} // namespace cis

namespace db 
{

const char* root_dir = nullptr;

void set_root_dir(const char* path)
{
    root_dir = path;
}

const char* get_root_dir()
{
    return root_dir;
}

} // namespace cis
