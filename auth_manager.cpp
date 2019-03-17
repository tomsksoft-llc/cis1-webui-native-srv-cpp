#include "auth_manager.h"

#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

#include "file_util.h"
#include "dirs.h"

constexpr const char* users_file_path = "/users.pwd";
constexpr const char* tokens_file_path = "/tokens.pwd";

auth_manager::auth_manager()
{
    std::string db_path = db::get_root_dir();
    load_users(path_cat(db_path, users_file_path));
    load_tokens(path_cat(db_path, tokens_file_path));
}

web_app::handle_result auth_manager::operator()(
        web_app::request_t& req,
        web_app::queue_t& queue,
        web_app::context_t& ctx)
{
    if(auto& cookies = 
            std::any_cast<std::map<std::string, std::string>&>(ctx["cookies"]);
            cookies.count("token"))
    {
        auto user = authenticate(cookies["token"]);
        if(!user.empty())
        {
            ctx["user"] = user;
        }
    }
    return web_app::handle_result::next;
}

std::string auth_manager::authenticate(const std::string& user, const std::string& pass)
{
    if(auto it = users_.find(user);
            it != users_.cend() && it->second == pass)
    {
        return "SomeToken";
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
    //TODO document.HasParseErrors() ...
    for(auto& member : document.GetObject())
    {
        users_[member.name.GetString()] = member.value.GetString();
    }
}

void auth_manager::load_tokens(std::filesystem::path tokens_file)
{
    std::ifstream tokens_db(tokens_file);
    rapidjson::IStreamWrapper isw(tokens_db);
    rapidjson::Document document;
    document.ParseStream(isw);
    //TODO document.HasParseErrors() ...
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
    for(auto& [user, pass] : users_)
    {
        key.SetString(user.c_str(), user.length(), document.GetAllocator());
        value.SetString(pass.c_str(), pass.length(), document.GetAllocator());
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
