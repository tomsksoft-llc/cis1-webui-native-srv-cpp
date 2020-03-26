/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>
#include <optional>

#include "sqlite_orm/sqlite_orm.h"

#include "init.h"
#include "database_structures.h"

namespace database
{

struct detail
{
    static decltype(auto) make_database(const char* path)
    {
        using namespace sqlite_orm;
        return make_storage(
                path,
                make_table("users",
                    make_column("user_id", &user::id, autoincrement(), primary_key()),
                    make_column("user_email", &user::email, unique()),
                    make_column("user_pass", &user::pass),
                    make_column("admin", &user::admin)),
                make_table("projects",
                    make_column("project_id", &project::id, autoincrement(), primary_key()),
                    make_column("project_name", &project::name, unique()),
                    make_column("project_deleted", &project::deleted)),
                make_table("project_user_rights",
                    make_column("project_user_right_id", &project_user_right::id, autoincrement(), primary_key()),
                    make_column("project_user_right_project_id", &project_user_right::project_id),
                    make_column("project_user_right_user_id", &project_user_right::user_id),
                    make_column("project_user_right_read", &project_user_right::read),
                    make_column("project_user_right_write", &project_user_right::write),
                    make_column("project_user_right_execute", &project_user_right::execute),
                    foreign_key(&project_user_right::project_id).references(&project::id).on_delete.cascade(),
                    foreign_key(&project_user_right::user_id).references(&user::id).on_delete.cascade()),
                make_table("tokens",
                    make_column("token_id", &token::id, autoincrement(), primary_key()),
                    make_column("token_user_id", &token::user_id),
                    make_column("token_value", &token::value, unique()),
                    make_column("token_expiration_time", &token::expiration_time),
                    foreign_key(&token::user_id).references(&user::id).on_delete.cascade()),
                make_table("api_access_keys",
                    make_column("api_access_key_id", &api_access_key::id, autoincrement(), primary_key()),
                    make_column("api_access_key_user_id", &api_access_key::user_id, unique()),
                    make_column("api_access_key_value", &api_access_key::value, unique()),
                    foreign_key(&api_access_key::user_id).references(&user::id).on_delete.cascade()));
    }
};

template <class T>
class database_transanction_guard
{
public:
    database_transanction_guard(T& db)
        : db_(db)
    {
        db_.begin_transaction();
    }

    ~database_transanction_guard()
    {
        rollback();
    }

    T* operator->()
    {
        return &db_;
    }

    T& operator*()
    {
        return db_;
    }

    void commit()
    {
        if(rollback_)
        {
            db_.commit();
            rollback_ = false;
        }
    }

    void rollback()
    {
        if(rollback_)
        {
            db_.rollback();
            rollback_ = false;
        }
    }
private:
    bool rollback_ = true;
    T& db_;
};

class database_wrapper
{
protected:
    struct private_constructor_delegate_t;

public:
    static std::unique_ptr<database_wrapper> create(
            const std::filesystem::path& path,
            user_credentials* admin_credentials,
            std::error_code& ec);

    template <class... Args>
    database_wrapper(
            const private_constructor_delegate_t&,
            Args&&... args)
        : database_wrapper(std::forward<Args>(args)...)
    {}

    decltype(detail::make_database(""))& get();

    database_transanction_guard<decltype(detail::make_database(""))> make_transaction();

    void sync();

protected:
    struct private_constructor_delegate_t
    {
        explicit private_constructor_delegate_t();
    };

private:
    decltype(detail::make_database("")) db_;

    database_wrapper(
            const std::filesystem::path& path);

    void init(const user_credentials &admin_credentials);
};

} // namespace database
