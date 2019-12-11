/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "database.h"

using namespace sqlite_orm;

namespace database
{

database_wrapper::private_constructor_delegate_t::private_constructor_delegate_t()
{}

std::unique_ptr<database_wrapper> database_wrapper::create(
        const std::filesystem::path& path,
        user_credentials* admin_credentials,
        std::error_code& ec)
{
    try
    {
        auto db = std::make_unique<database_wrapper>(
                private_constructor_delegate_t{},
                path);

        db->sync();

        if(admin_credentials != nullptr)
        {
            db->init(admin_credentials->name,
                    admin_credentials->email,
                    admin_credentials->pass);
        }

        return db;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return nullptr;
    }
}

database_wrapper::database_wrapper(
        const std::filesystem::path& path)
    : db_(detail::make_database(path.string().c_str()))
{}

void database_wrapper::sync()
{
    db_.sync_schema();
}

decltype(detail::make_database(""))& database_wrapper::get()
{
    return db_;
}

database_transanction_guard<decltype(detail::make_database(""))>
database_wrapper::make_transaction()
{
    return {db_};
}

void database_wrapper::init(
        const std::string& username,
        const std::string& email,
        const std::string& password)
{
    auto db = make_transaction();

    db->insert(group{-1, "user"});
    db->insert(group{-1, "admin"});

    intmax_t admin_group_id = db->last_insert_rowid();

    db->insert(
            user{
            -1,
            admin_group_id,
            username,
            email,
            password});

    db->insert(permission{-1, "users.list"});
    db->insert(group_permission{
            -1,
            admin_group_id,
            (intmax_t)db->last_insert_rowid()});

    db->insert(permission{-1, "users.permissions"});
    db->insert(group_permission{
            -1,
            admin_group_id,
            (intmax_t)db->last_insert_rowid()});

    db->insert(permission{-1, "users.change_group"});
    db->insert(group_permission{
            -1,
            admin_group_id,
            (intmax_t)db->last_insert_rowid()});

    db.commit();
}

} // namespace database
