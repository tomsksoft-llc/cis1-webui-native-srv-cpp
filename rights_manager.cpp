#include "rights_manager.h"

#include <fstream>

#include <boost/property_tree/ptree.hpp> //TODO Replace with RapidJSON
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

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

std::optional<bool> rights_manager::check_right(std::string username, std::string resource_name)
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
    return std::nullopt;
}

void rights_manager::save_to_file(const std::filesystem::path& file)
{
    pt::ptree tree;
    pt::ptree resources_node;
    for(auto& resource : resources_)
    {
        resources_node.put(pt::ptree::path_type(resource.first, '\0'), resource.second);
    }
    tree.add_child("resources", resources_node);

    pt::ptree user_rights_node;
    for(auto& user : user_rights_)
    {
        pt::ptree user_node;
        for(auto& resource : user.second)
        {
            user_node.put(pt::ptree::path_type(resource.first, '\0'), resource.second);
        }
        user_rights_node.add_child(user.first, user_node);
    }
    tree.add_child("user_rights", user_rights_node);
    std::ofstream ofs(file);
    pt::write_json(ofs, tree);
}

void rights_manager::load_from_file(const std::filesystem::path& file)
{
    pt::ptree tree;
    {
        std::ifstream ifs(file);
        pt::read_json(ifs, tree);
    }

    auto resources_node = tree.get_child("resources");
    for(auto& resource_node : resources_node)
    {
        resources_[resource_node.first] = 
                resource_node.second.get_value<bool>();
    }

    auto user_rights_node = tree.get_child("user_rights");
    for(auto& user_node : user_rights_node)
    {
        auto [it, success] = user_rights_.try_emplace(user_node.first);
        for(auto& right_node : user_node.second)
        {
            it->second[right_node.first] = 
                right_node.second.get_value<bool>();
        }
    }
}
