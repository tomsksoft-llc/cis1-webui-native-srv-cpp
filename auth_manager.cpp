#include "auth_manager.h"

#include <chrono>
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

handle_result auth_manager::operator()(
        http::request<http::string_body>& /*req*/,
        http_session::queue& /*queue*/,
        request_context& ctx)
{
    if(const auto& cookies = ctx.cookies; cookies.count("token"))
    {
        auto user = authenticate(cookies.at("token"));
        if(!user.empty())
        {
            ctx.username = user;
        }
    }
    return handle_result::next;
}

std::string auth_manager::authenticate(const std::string& user, const std::string& pass)
{
    if(auto it = users_.find(user);
            it != users_.cend() && it->second == pass)
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
    //TODO document.HasParseErrors() ...
    if(!document.IsObject())
    {
        return;
    }
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
