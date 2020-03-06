/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

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
#include "exceptions/load_config_error.h"
#include "openssl_wrapper/openssl_wrapper.h"
#include "base64.h"

using namespace sqlite_orm;
using namespace database;

auth_manager::auth_manager(
        boost::asio::io_context& ioc,
        database::database_wrapper& db)
    : db_(db)
    , timer_(ioc)
{
    cleanup();
}

std::optional<std::string> auth_manager::authenticate(
        const std::string& email,
        const std::string& pass,
        std::error_code& ec)
{
    try
    {
        auto db = db_.make_transaction();

        auto ids = db->select(
                &user::id,
                where(c(&user::email) == email
                        && c(&user::pass) == pass));

        if(ids.size() == 1)
        {
            uint64_t expiration_time = (std::chrono::seconds(std::time(nullptr))
                                      + std::chrono::hours(24*7)).count();

            std::string token_str;
            for(bool generated = false; !generated;)
            {
                std::array<unsigned char, 32> token_bytes;
                openssl::rand(token_bytes.data(), token_bytes.size());
                token_str = base64_encode(token_bytes.data(), token_bytes.size());

                try
                {
                    db->insert(token{-1, ids[0], token_str, expiration_time});
                    generated = true;
                }
                catch(const std::system_error& ec)
                {}
            }

            db.commit();
            return token_str;
        }

        return std::nullopt;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return std::nullopt;
    }
}

std::optional<std::string> auth_manager::authenticate(
        const std::string& token_value,
        std::error_code& ec)
{
    try
    {
        auto db = db_.make_transaction();

        auto ids = db->select(
                columns(&token::user_id, &token::expiration_time),
                where(c(&token::value) == token_value));

        if(ids.size() == 1)
        {
            auto current_time = std::chrono::seconds(std::time(nullptr)).count();

            if(std::get<1>(ids[0]) < current_time)
            {
                db->remove_all<token>(where(c(&token::value) == token_value));

                db.commit();
                return std::nullopt;
            }

            auto users = db->select(
                    &user::name,
                    where(c(&user::id) == std::get<0>(ids[0])));

            db.commit();

            return users[0];
        }

        return std::nullopt;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return std::nullopt;
    }
}

bool auth_manager::has_user(
        const std::string& email,
        std::error_code& ec) const
{
    try
    {
        auto db = db_.make_transaction();

        auto users_count = db->count<user>(where(c(&user::email) == email));

        if(users_count)
        {
            db.commit();
            return true;
        }

        return false;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return false;
    }
}

std::optional<std::string> auth_manager::get_group(
        const std::string& username,
        std::error_code& ec) const
{
    try
    {
        auto db = db_.make_transaction();

        auto groups = db->select(
                &group::name,
                inner_join<user>(on(c(&user::group_id) == &group::id)),
                where(c(&user::name) == username));

        if(groups.size() == 1)
        {
            db.commit();
            return groups[0];
        }

        return std::nullopt;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return std::nullopt;
    }
}

std::optional<std::string> auth_manager::generate_api_key(
        const std::string& email,
        std::error_code& ec)
{
    try
    {
        auto db = db_.make_transaction();
        auto ids = db->select(
                &user::id,
                where(c(&user::email) == email));

        if(ids.size() == 1)
        {
            auto unix_timestamp = std::chrono::seconds(std::time(nullptr));
            std::ostringstream os;
            os << unix_timestamp.count();
            std::string key_str = email + os.str() + "SALT";
            os.clear();

            static const std::hash<std::string> hash_fn;
            os << hash_fn(key_str);
            try
            {
                auto insertedId = db->insert(api_access_key{-1, ids[0], os.str()});
            }
            catch(std::system_error& err)
            {
                return std::nullopt;
            }

            db.commit();
            return os.str();
        }

        return std::nullopt;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return std::nullopt;
    }
}

std::optional<std::string> auth_manager::get_api_key(
        const std::string& name,
        std::error_code& ec)
{
    try
    {
        auto db = db_.make_transaction();
        auto values = db->select(
                &api_access_key::value,
                inner_join<user>(on(c(&user::id) == &api_access_key::user_id)),
                where(c(&user::name) == name));

        if(values.size() == 1)
        {
            db.commit();
            return values[0];
        }

        return std::nullopt;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return std::nullopt;
    }
}

bool auth_manager::remove_api_key(
        const std::string& name,
        std::error_code& ec)
{
    try
    {
        auto db = db_.make_transaction();
        auto ids = db->select(
                &api_access_key::id,
                inner_join<user>(on(c(&user::id) == &api_access_key::user_id)),
                where(c(&user::name) == name));
        if(ids.size() == 1)
        {
            db->remove<api_access_key>(ids[0]);
            db.commit();

            return true;
        }

        return false;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return false;
    }
}

bool auth_manager::change_pass(
        const std::string& email,
        const std::string& old_pass,
        const std::string& new_pass,
        std::error_code& ec)
{
    try
    {
        if(new_pass.empty())
        {
            return false;
        }

        auto db = db_.make_transaction();

        auto users = db->select(
                &user::id,
                where(c(&user::email) == email
                        && c(&user::pass) == old_pass));

        if(users.size() == 1)
        {
            db->update_all(
                    set(assign(&user::pass, new_pass)),
                    where(c(&user::id) == users[0]));

            db.commit();
            return true;
        }

        return false;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return false;
    }
}

std::vector<user> auth_manager::get_users(
        std::error_code& ec) const
{
    try
    {
        return db_.get().get_all<user>();
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return {};
    }
}

std::optional<user_info> auth_manager::get_user_info(
        const std::string& username,
        std::error_code& ec) const
{
    try
    {
        auto db = db_.make_transaction();

        auto users = db->select(
                columns(&user::id,
                        &user::name,
                        &user::email,
                        &group::name),
                inner_join<user>(on(c(&user::group_id) == &group::id)),
                where(c(&user::name) == username));

        if(users.size() != 1)
        {
            return std::nullopt;
        }

        auto api_access_keys = db->select(
                &api_access_key::value,
                where(c(&api_access_key::user_id) == std::get<0>(users[0])));

        std::optional<std::string> key;

        if(api_access_keys.size() == 1)
        {
            key = api_access_keys[0];
        }

        db.commit();

        return user_info{
                std::get<1>(users[0]),
                std::get<2>(users[0]),
                std::get<3>(users[0]),
                key};
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return std::nullopt;
    }
}

std::vector<user_info> auth_manager::get_user_infos(
        std::error_code& ec) const
{
    try
    {
        std::vector<user_info> result;

        auto db = db_.make_transaction();

        auto users = db->select(
                columns(&user::id,
                        &user::name,
                        &user::email,
                        &group::name),
                inner_join<user>(on(c(&user::group_id) == &group::id)));

        result.resize(users.size());

        size_t i = 0;
        for(auto [id, name, email, group] : users)
        {
            std::optional<std::string> key;
            auto api_access_keys = db->select(
                    &api_access_key::value,
                    where(c(&api_access_key::user_id) == id));
            if(api_access_keys.size() == 1)
            {
                key = api_access_keys[0];
            }
            result[i] = {name, email, group, key};
            ++i;
        }

        db.commit();

        return result;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return {};
    }
}

std::optional<database::group> auth_manager::get_group_info(
        const std::string& group_name,
        std::error_code& ec) const
{
    try
    {
        auto db = db_.make_transaction();

        auto groups = db->select(
                &group::id,
                where(c(&group::name) == group_name));

        if(groups.size() != 1)
        {
            return std::nullopt;
        }

        const auto group_id = groups[0];
        return database::group{group_id, group_name};
    }
    catch(const std::system_error& e)
    {
        ec = e.code();
        return std::nullopt;
    }
}

bool auth_manager::delete_token(
        const std::string& token_value,
        std::error_code& ec)
{
    try
    {
        auto db = db_.make_transaction();

        db->remove_all<token>(
                where(c(&token::value) == token_value));

        db.commit();

        return true;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return false;
    }
}

bool auth_manager::add_user(
        const std::string& email,
        const std::string& pass,
        bool admin,
        std::error_code& ec)
{
    try
    {
        auto db = db_.make_transaction();

        auto users_count = db->count<user>(
                where(c(&user::email) == email));

        if(users_count == 0)
        {
            db->insert(user{-1, email, pass, admin});

            db.commit();

            return true;
        }

        return false;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return false;
    }
}

void auth_manager::cleanup()
{
    auto db = db_.make_transaction();

    auto current_time = std::chrono::seconds(std::time(nullptr)).count();

    db->remove_all<token>(where(c(&token::expiration_time) < current_time));

    db.commit();

    timer_.expires_after(std::chrono::hours(12));

    timer_.async_wait(
            [&](const boost::system::error_code& error)
            {
                if(!error)
                {
                    cleanup();
                }
            });
}
