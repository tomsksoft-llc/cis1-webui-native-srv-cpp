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

std::optional<bool> rights_manager::check_user_permission(
        const std::string& username,
        const std::string& permission_name,
        std::error_code& ec) const
{
    try
    {
        auto db = db_.make_transaction();

        auto groups = db->select(
                &user::group_id,
                where(c(&user::name) == username));

        auto permissions = db->select(
                &permission::id,
                where(c(&permission::name) == permission_name));

        if(groups.size() == 1 && permissions.size() == 1)
        {
            auto group_permission_count = db->count<group_permission>(
                    where(c(&group_permission::group_id)      == groups[0]
                       && c(&group_permission::permission_id) == permissions[0]));

            if(group_permission_count == 1)
            {
                db.commit();

                return true;
            }
        }

        return false;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();

        return std::nullopt;
    }
}

std::optional<project_user_right> rights_manager::check_project_right(
        const std::string& username,
        const std::string& projectname,
        std::error_code& ec) const
{
    try
    {
        auto db = db_.make_transaction();

        auto users = db->select(
                &user::id,
                where(c(&user::name) == username));

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
        const std::string& username,
        std::error_code& ec) const
{
    try
    {
        std::map<std::string, project_rights> result;

        auto db = db_.make_transaction();

        auto users = db->select(
                &user::id,
                where(c(&user::name) == username));

        if(users.size() == 1)
        {
            auto projects = db->select(&project::name);

            for(auto& project_name : projects)
            {
                result.insert({project_name, project_rights{true, true, true}});
            }

            auto projects_rights = db->select(
                    columns(&project::name,
                            &project_user_right::read,
                            &project_user_right::write,
                            &project_user_right::execute),
                            inner_join<project>(
                                    on(c(&project::id) == &project_user_right::project_id)),
                            where(c(&project_user_right::user_id) == users[0]));

            for(auto& [project_name, read, write, execute] : projects_rights)
            {
                result[project_name].read = read;
                result[project_name].write = write;
                result[project_name].execute = execute;
            }
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
        const std::string& username,
        const std::string& projectname,
        project_user_right rights,
        std::error_code& ec)
{
    try
    {
        auto db = db_.make_transaction();

        auto users = db->select(
                &user::id,
                where(c(&user::name) == username));

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

bool rights_manager::set_group_projects_permissions(
        intmax_t group_id,
        const project_rights& rights,
        std::error_code& ec)
{
    try
    {
        auto db = db_.make_transaction();

        projects_group_right group_rights{-1,
                                          group_id,
                                          rights.read,
                                          rights.write,
                                          rights.execute};

        auto ids = db->select(&projects_group_right::id,
                              where(c(&projects_group_right::group_id) == group_id));

        if(ids.size() == 1)
        {
            group_rights.id = ids[0];
            db->replace(group_rights);
        }
        else if(ids.empty())
        {
            //the group_rights.id is -1 already
            db->insert(group_rights);
        }
        else
        {
            // there should be no other rows for the group
            return false;
        }

        db.commit();
        return true;
    }
    catch(const std::system_error& e)
    {
        ec = e.code();
        return false;
    }
}
