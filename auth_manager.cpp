#include "auth_manager.h"

#include <chrono>
#include <fstream>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/error/en.h>

#include "file_util.h"
#include "dirs.h"
#include "load_config_error.h"

constexpr const char* users_file_path = "/users.pwd";
constexpr const char* tokens_file_path = "/tokens.pwd";

void save_user(
        const user& user,
        rapidjson::Value& user_value,
        rapidjson::Document::AllocatorType& allocator)
{
    user_value.SetObject();
    rapidjson::Value value;
    value.SetString(user.pass.c_str(), user.pass.length(), allocator);
    user_value.AddMember("pass", value, allocator);
    value.SetString(user.email.c_str(), user.email.length(), allocator);
    user_value.AddMember("email", value, allocator);
    value.SetBool(user.admin);
    user_value.AddMember("admin", value, allocator);
    value.SetBool(user.disabled);
    user_value.AddMember("disabled", value, allocator);
    if(user.api_access_key)
    {
        auto& key = user.api_access_key.value();
        value.SetString(key.c_str(), key.length(), allocator);
        user_value.AddMember("api_access_key", value, allocator);
    }
}

user load_user(const rapidjson::Value& value)
{
    user result;
    result.pass = value["pass"].GetString();
    result.email = value["email"].GetString();
    result.admin = value["admin"].GetBool();
    result.disabled = value["disabled"].GetBool();
    if(value.HasMember("api_access_key"))
    {
        result.api_access_key = value["api_access_key"].GetString();
    }
    return result;
}

auth_manager::auth_manager()
{
    std::string db_path = db::get_root_dir();
    load_users(path_cat(db_path, users_file_path));
    load_tokens(path_cat(db_path, tokens_file_path));
}

std::string auth_manager::authenticate(const std::string& user, const std::string& pass)
{
    if(auto it = users_.find(user);
            it != users_.cend() && it->second.pass == pass)
    {
        auto unix_timestamp = std::chrono::seconds(std::time(NULL));
        std::ostringstream os;
        os << unix_timestamp.count();
        std::string token = user + os.str();
        os.clear();
        static const std::hash<std::string> hash_fn;
        os << hash_fn(token);
        tokens_[os.str()] = user;
        save_on_disk();
        return os.str();
    }
    return "";
}

std::string auth_manager::authenticate(const std::string& token)
{
    if(auto it = tokens_.find(token); it != tokens_.cend())
    {
        return it->second;
    }
    return "";
}

bool auth_manager::has_user(const std::string& name)
{
    if(auto it = users_.find(name);
            it != users_.cend())
    {
        return true;
    }
    return false;
}

void auth_manager::set_disabled(const std::string& name, bool state)
{
    if(auto it = users_.find(name);
            it != users_.cend())
    {
        it->second.admin = state;
    }
}

void auth_manager::make_admin(const std::string& name, bool state)
{
    if(auto it = users_.find(name);
            it != users_.cend())
    {
        it->second.disabled = state;
    }
}

bool auth_manager::is_admin(const std::string& name)
{
    if(auto it = users_.find(name);
            it != users_.cend())
    {
        return it->second.admin;
    }
    return false;
}

std::optional<std::string> auth_manager::generate_api_key(const std::string& name)
{
    if(auto it = users_.find(name);
            it != users_.cend())
    {
        if(!it->second.api_access_key)
        {
            auto unix_timestamp = std::chrono::seconds(std::time(NULL));
            std::ostringstream os;
            os << unix_timestamp.count();
            std::string token = name + it->second.pass + os.str() + "SALT";
            os.clear();
            static const std::hash<std::string> hash_fn;
            os << hash_fn(token);

            it->second.api_access_key = os.str();
        }
        return it->second.api_access_key;
    }
    return std::nullopt;
}

bool auth_manager::change_pass(
        const std::string& user,
        const std::string& old_pass,
        const std::string& new_pass)
{
    if(new_pass.empty())
    {
        return false;
    }
    if(auto it = users_.find(user);
            it != users_.cend() && it->second.pass == old_pass)
    {
        it->second.pass = new_pass;
        save_on_disk();
        return true;
    }
    return false;
}

const std::map<std::string, user>& auth_manager::get_users()
{
    return users_;
}

void auth_manager::delete_token(const std::string& token)
{
    tokens_.erase(token);
}

std::optional<std::string> auth_manager::add_user(
        std::string user,
        std::string pass)
{
    return std::nullopt;
}

void auth_manager::save_on_disk()
{
    std::string db_path = db::get_root_dir();
    save_users(path_cat(db_path, users_file_path));
    save_tokens(path_cat(db_path, tokens_file_path));
}

void auth_manager::load_users(std::filesystem::path users_file)
{
    std::ifstream users_db(users_file);
    rapidjson::IStreamWrapper isw(users_db);
    rapidjson::Document document;
    document.ParseStream(isw);
    if(document.HasParseError())
    {
        std::stringstream ss;
        ss << "Can't load users db: " << GetParseError_En(document.GetParseError());
        throw load_config_error(ss.str());
    }
    if(!document.IsObject())
    {
        return;
    }
    for(auto& member : document.GetObject())
    {
        users_[member.name.GetString()] = load_user(member.value);
    }
}

void auth_manager::load_tokens(std::filesystem::path tokens_file)
{
    std::ifstream tokens_db(tokens_file);
    rapidjson::IStreamWrapper isw(tokens_db);
    rapidjson::Document document;
    document.ParseStream(isw);
    if(document.HasParseError())
    {
        std::stringstream ss;
        ss << "Can't load tokens db: " << GetParseError_En(document.GetParseError());
        throw load_config_error(ss.str());
    }
    if(!document.IsObject())
    {
        return;
    }
    for(auto& member : document.GetObject())
    {
        tokens_[member.name.GetString()] = member.value.GetString();
    }
}

void auth_manager::save_users(std::filesystem::path users_file)
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Value key;
    rapidjson::Value value;
    for(auto& [username, user] : users_)
    {
        key.SetString(username.c_str(), username.length(), document.GetAllocator());
        save_user(user, value, document.GetAllocator());
        document.AddMember(key, value, document.GetAllocator());
    }
    std::ofstream users_db(users_file);
    rapidjson::OStreamWrapper osw(users_db);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    document.Accept(writer);
}

void auth_manager::save_tokens(std::filesystem::path tokens_file)
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Value key;
    rapidjson::Value value;
    for(auto& [token, user] : tokens_)
    {
        key.SetString(token.c_str(), token.length(), document.GetAllocator());
        value.SetString(user.c_str(), user.length(), document.GetAllocator());
        document.AddMember(key, value, document.GetAllocator());
    }
    std::ofstream tokens_db(tokens_file);
    rapidjson::OStreamWrapper osw(tokens_db);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    document.Accept(writer);
}
