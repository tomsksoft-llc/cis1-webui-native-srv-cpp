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
        user_credentials* guest_credentials,
        std::error_code& ec)
{
    try
    {
        auto db = std::make_unique<database_wrapper>(
                private_constructor_delegate_t{},
                path);

        db->sync();

        if(admin_credentials || guest_credentials)
        {
            db->init(admin_credentials, guest_credentials);
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
        user_credentials *admin_credentials,
        user_credentials *guest_credentials)
{
    auto db = make_transaction();

    const auto insert_sys_user_fn
            = [&db](intmax_t group_id, const user_credentials& credentials)
            {
                db->insert(
                        user{
                                -1,
                                group_id,
                                credentials.name,
                                credentials.email,
                                credentials.pass});
            };

    db->insert(group{-1, "user"});
    intmax_t user_group_id = db->last_insert_rowid();

    db->insert(group{-1, "admin"});
    intmax_t admin_group_id = db->last_insert_rowid();

    db->insert(group{-1, "guest"});
    intmax_t guest_group_id = db->last_insert_rowid();

    if(admin_credentials)
    {
        insert_sys_user_fn(admin_group_id, *admin_credentials);
    }

    if(guest_credentials)
    {
        insert_sys_user_fn(guest_group_id, *guest_credentials);
    }

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

    db->insert(permission{-1, "users.add"});
    db->insert(group_permission{
            -1,
            admin_group_id,
            (intmax_t)db->last_insert_rowid()});

    db->insert(permission{-1, "groups.projects.permissions"});
    db->insert(group_permission{
            -1,
            admin_group_id,
            (intmax_t)db->last_insert_rowid()});

    db.commit();

    db->insert(group_default_rights{
            -1,
            user_group_id,
            true, // read
            false, // write
            true // execute
    });

    db->insert(group_default_rights{
            -1,
            admin_group_id,
            true, // read
            true, // write
            true // execute
    });

    db->insert(group_default_rights{
            -1,
            guest_group_id,
            true, // read
            false, // write
            false // execute
    });
}

} // namespace database
