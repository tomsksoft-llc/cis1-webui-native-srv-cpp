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

        return false;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return false;
    }
}

std::optional<project_user_right> rights_manager::check_project_right(
        const std::string& email,
        const std::string& projectname,
        std::error_code& ec) const
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
            auto rights = db->get_all<project_user_right>(
                    where(c(&project_user_right::user_id)    == users[0]
                       && c(&project_user_right::project_id) == projects[0]));

            if(rights.size() == 1)
            {
                db.commit();

                return rights[0];
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
                &user::id,
                where(c(&user::email) == email));
        if(users.size() != 1)
        {
            db.commit();
            return result;
        }

        // get project list and fill the result with the projects and default permissions
        auto projects = db->select(&project::name);

        for(auto& project_name : projects)
        {
            result.insert({project_name, project_rights{false, false, false}});
        }

        // set the result's permissions with the following project_rights
        auto projects_rights = db->select(
                columns(&project::name,
                        &project_user_right::read,
                        &project_user_right::write,
                        &project_user_right::execute),
                inner_join<project>(
                        on(c(&project::id) == &project_user_right::project_id)),
                where(c(&project_user_right::user_id) == users[0]));

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
