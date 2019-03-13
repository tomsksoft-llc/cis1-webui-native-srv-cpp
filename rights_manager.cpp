#include "rights_manager.h"

void rights_manager::add_resource(std::string resource_name, bool default_value)
{
    resources_.try_emplace(resource_name, default_value);
}

void rights_manager::set_right(
        std::string username,
        std::string resource_name,
        bool value)
{
    user_rights_[username][resource_name] = value;
}

bool rights_manager::check_right(std::string username, std::string resource_name)
{
    if(auto user = user_rights_.find(username); user != user_rights_.end())
    {
        if(auto resource = user->second.find(resource_name);
                resource != user->second.end())
        {
            return resource->second;
        }
    }
    if(auto resource = resources_.find(resource_name); resource != resources_.end())
    {
        return resource->second;
    }
    //TODO throw or optional
}
