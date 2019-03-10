#include "cis_dirs.h"

const char* root_dir = nullptr;

void cis::set_root_dir(const char* path)
{
    root_dir = path;
}

const char* cis::get_root_dir()
{
    return root_dir;
}
