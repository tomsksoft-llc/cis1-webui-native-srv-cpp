#include "database.h"

#include <chrono>
#include <fstream>
#include <sstream>

using namespace sqlite_orm;

namespace database
{

database::database(const std::filesystem::path& path, std::optional<admin_user> admin)
    : db_(detail::make_database(path.c_str()))
{
    sync();

    if(admin)
    {
        init(admin->name, admin->email, admin->pass);
    }
}

void database::sync()
{
    db_.sync_schema();
}

decltype(detail::make_database(""))& database::get()
{
    return db_;
}

database_transanction_guard<decltype(detail::make_database(""))>
database::make_transaction()
{
    return {db_};
}

void database::init(
        const std::string& username,
        const std::string& email,
        const std::string& password)
{
    auto db = make_transaction();

    db->insert(group{-1, "user"});
    db->insert(group{-1, "admin"});

    ssize_t admin_group_id = db->last_insert_rowid();

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
            (ssize_t)db->last_insert_rowid()});

    db->insert(permission{-1, "users.permissions"});
    db->insert(group_permission{
            -1,
            admin_group_id,
            (ssize_t)db->last_insert_rowid()});

    db->insert(permission{-1, "users.change_group"});
    db->insert(group_permission{
            -1,
            admin_group_id,
            (ssize_t)db->last_insert_rowid()});

    db.commit();
}

} // namespace database
