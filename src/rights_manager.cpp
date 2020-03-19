/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "rights_manager.h"

using namespace database;
using namespace sqlite_orm;

rights_manager::rights_manager(
        boost::asio::io_context& ioc,
        database::database_wrapper& db)
    : ioc_(ioc)
    , db_(db)
{}

bool rights_manager::is_admin(
        const std::string& email,
        std::error_code& ec) const
{
    try
    {
        auto db = db_.make_transaction();

        auto admin_fields = db->select(
                &user::admin,
                where(c(&user::email) == email));

        if(admin_fields.size() != 1)
        {
            return false;
        }

        return admin_fields[0];
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return false;
    }
}

bool rights_manager::set_admin_status(
        const std::string& email,
        bool admin,
        std::error_code& ec) const
{
    try
    {
        auto db = db_.make_transaction();

        auto users = db->get_all<user>(
                where(c(&user::email) == email));

        if(users.size() != 1)
        {
            return false;
        }

        users[0].admin = admin;

        db->update(users[0]);

        db.commit();

        return true;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return false;
    }
}

std::optional<project_rights> rights_manager::check_project_right(
        const std::string& email,
        const std::string& projectname,
        std::error_code& ec) const
{
    try
    {
        auto db = db_.make_transaction();

        auto users = db->select(
                columns(&user::id,
                        &user::admin),
                where(c(&user::email) == email));

        auto projects = db->select(
                &project::id,
                where(c(&project::name) == projectname));

        if(users.size() == 1 && projects.size() == 1)
        {
            const auto &[user_id, user_admin] = users[0];

            if(user_admin)
            {
                db.commit();

                return project_rights{true, true, true};
            }

            auto rights = db->get_all<project_user_right>(
                    where(c(&project_user_right::user_id) == user_id
                          && c(&project_user_right::project_id) == projects[0]));

            if(rights.size() == 1)
            {
                db.commit();

                return project_rights{rights[0].read, rights[0].write, rights[0].execute};
            }
        }

        return std::nullopt;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return std::nullopt;
    }
}

std::map<std::string, project_rights> rights_manager::get_permissions(
        const std::string& email,
        std::error_code& ec) const
{
    try
    {
        std::map<std::string, project_rights> result;

        auto db = db_.make_transaction();

        // get user_id by the username
        auto users = db->select(
                columns(&user::id,
                        &user::admin),
                where(c(&user::email) == email));
        if(users.size() != 1)
        {
            db.commit();
            return result;
        }

        const auto &[user_id, user_admin] = users[0];

        // get project list and fill the result with the projects and default permissions
        auto projects = db->select(&project::name);

        auto default_rights = user_admin
                              ? project_rights{true, true, true}
                              : project_rights{false, false, false};

        for(auto& project_name : projects)
        {
            result.insert({project_name, default_rights});
        }

        if(user_admin)
        {
            db.commit();
            return result;
        }

        // set the result's permissions with the following project_rights
        auto projects_rights = db->select(
                columns(&project::name,
                        &project_user_right::read,
                        &project_user_right::write,
                        &project_user_right::execute),
                inner_join<project>(
                        on(c(&project::id) == &project_user_right::project_id)),
                where(c(&project_user_right::user_id) == user_id));

        for(auto&[project_name, read, write, execute] : projects_rights)
        {
            result[project_name].read = read;
            result[project_name].write = write;
            result[project_name].execute = execute;
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

std::map<std::string, project_rights_ex> rights_manager::get_permissions_by_project(
        const std::string& project,
        std::error_code& ec) const
{
    try
    {
        std::map<std::string, project_rights_ex> result;

        auto db = db_.make_transaction();

        auto projects = db->select(
                &project::id,
                where(c(&project::name) == project));

        if(projects.size() != 1)
        {
            db.commit();
            return result;
        }

        auto users = db->select(
                columns(&user::id,
                        &user::email,
                        &user::admin));

        // set default rights for the users
        for(const auto&[id, email, admin] : users)
        {
            result[email] = admin
                            ? project_rights_ex{true, true, true, true}
                            : project_rights_ex{false, false, false, false};
        }

        auto users_rights = db->select(
                columns(&user::email,
                        &project_user_right::read,
                        &project_user_right::write,
                        &project_user_right::execute),
                inner_join<user>(
                        on(c(&user::id) == &project_user_right::user_id)),
                where(c(&project_user_right::project_id) == projects[0]));

        for(auto&[email, read, write, execute] : users_rights)
        {
            project_rights_ex& rights = result[email];
            if(rights.admin)
            {
                // admin has all rights always
                continue;
            }

            rights.read = read;
            rights.write = write;
            rights.execute = execute;
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

bool rights_manager::set_user_project_permissions(
        const std::string& email,
        const std::string& projectname,
        project_user_right rights,
        std::error_code& ec)
{
    try
    {
        auto db = db_.make_transaction();

        auto users = db->select(
                &user::id,
                where(c(&user::email) == email));

        auto projects = db->select(
                &project::id,
                where(c(&project::name) == projectname));

        if(users.size() == 1 && projects.size() == 1)
        {
            rights.user_id = users[0];
            rights.project_id = projects[0];
            auto ids = db->select(&project_user_right::id,
                    where(c(&project_user_right::user_id)    == users[0]
                       && c(&project_user_right::project_id) == projects[0]));

            if(ids.size() == 1)
            {
                rights.id = ids[0];
                db->replace(rights);
            }
            else
            {
                rights.id = -1;
                db->insert(rights);
            }

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
