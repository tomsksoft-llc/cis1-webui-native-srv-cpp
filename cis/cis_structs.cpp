#include "cis_structs.h"

#include <utility>
#include <fstream>

namespace cis
{

// build

build::build(fs_cache_node<fs_mapper>* fs_node)
    : fs_node_(fs_node)
{}

void build::set_fs_node(
        fs_cache_node<fs_mapper>* fs_node)
{
    fs_node_ = fs_node;
}

std::pair<cis_entry_interface*, std::any> build::make_child(
        fs_cache_node<fs_mapper>* child_node)
{
    return {nullptr, {}};
}

void build::remove_child(std::any it)
{}

void build::add_output(
        fs_cache_node<fs_mapper>* fs_node)
{
    std::ifstream output_file(fs_node->dir_entry().path());
    std::string date;
    output_file >> date;
    info_.date = date.substr(0, 10);
}

void build::remove_output()
{
    info_.date = std::nullopt;
}

void build::add_exitcode(
        fs_cache_node<fs_mapper>* fs_node)
{
    std::ifstream exitcode_file(fs_node->dir_entry().path());
    int exitcode;
    exitcode_file >> exitcode;
    info_.status = exitcode;
}

void build::remove_exitcode()
{
    info_.status = std::nullopt;
}

void build::erase()
{
    fs_node_->remove();
}

const build_info& build::get_info() const
{
    return info_;
}

immutable_container_proxy<
        typename fs_cache_node<fs_mapper>::tree_t> build::get_files()
{
    return fs_node_->childs();
}

const fs_cache_node<fs_mapper>::tree_t& build::get_files() const
{
    return std::as_const(*fs_node_).childs();
}

// param

param::param(
        std::string param_name,
        std::string param_default_value)
    : name(param_name)
    , default_value(param_default_value)
{}

// job

job::job(fs_cache_node<fs_mapper>* fs_node)
    : fs_node_(fs_node)
{}

void job::set_fs_node(fs_cache_node<fs_mapper>* fs_node)
{
    fs_node_ = fs_node;
}

std::pair<cis_entry_interface*, std::any> job::make_child(
        fs_cache_node<fs_mapper>* child_node)
{
    auto [it, ok] = builds_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(child_node->filename()),
            std::forward_as_tuple(child_node));
    return {&(it->second), it};
}

void job::remove_child(std::any it)
{
    auto build_it = std::any_cast<std::map<std::string, build>::iterator>(it);
    builds_.erase(build_it);
}

void job::add_params(
        fs_cache_node<fs_mapper>* fs_node)
{
    std::ifstream params_file(fs_node->dir_entry().path());
    std::string param;
    std::string default_value;
    while(params_file.good())
    {
        std::getline(params_file, param, '=');
        std::getline(params_file, default_value, '\n');
        if(!default_value.empty())
        {
            default_value = default_value.substr(
                    1,
                    default_value.size() - 2);
        }
        params_.emplace_back(param, default_value);
    }
}

void job::remove_params()
{
    params_.clear();
}

void job::erase()
{
    fs_node_->remove();
}

immutable_container_proxy<std::map<std::string, build>> job::get_builds()
{
    return {builds_};
}

const std::map<std::string, build>& job::get_builds() const
{
    return builds_;
}

const std::vector<param>& job::get_params() const
{
    return params_;
}

immutable_container_proxy<
        typename fs_cache_node<fs_mapper>::tree_t> job::get_files()
{
    return fs_node_->childs();
}

const fs_cache_node<fs_mapper>::tree_t& job::get_files() const
{
    return std::as_const(*fs_node_).childs();
}

// project

project::project(fs_cache_node<fs_mapper>* fs_node)
    : fs_node_(fs_node)
{}

void project::set_fs_node(
        fs_cache_node<fs_mapper>* fs_node)
{
    fs_node_ = fs_node;
}

std::pair<cis_entry_interface*, std::any> project::make_child(
        fs_cache_node<fs_mapper>* child_node)
{
    auto [it, ok] = jobs_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(child_node->filename()),
            std::forward_as_tuple(child_node));
    return {&(it->second), {it}};
}

void project::remove_child(std::any it)
{
    auto job_it = std::any_cast<std::map<std::string, job>::iterator>(it);
    jobs_.erase(job_it);
}

void project::erase()
{
    fs_node_->remove();
}

immutable_container_proxy<
        std::map<std::string, job>> project::get_jobs()
{
    return {jobs_};
}

const std::map<std::string, job>& project::get_jobs() const
{
    return jobs_;
}

immutable_container_proxy<
        typename fs_cache_node<fs_mapper>::tree_t> project::get_files()
{
    return fs_node_->childs();
}

const fs_cache_node<fs_mapper>::tree_t& project::get_files() const
{
    return std::as_const(*fs_node_).childs();
}

// project_list

project_list::project_list(database::database& db)
    : db_(db)
{}

void project_list::set_fs_node(
        fs_cache_node<fs_mapper>* fs_node)
{
    fs_node_ = fs_node;
}

std::pair<cis_entry_interface*, std::any> project_list::make_child(
        fs_cache_node<fs_mapper>* child_node)
{
    std::string project_name = child_node->filename();
    {
        using namespace sqlite_orm;
        auto db = db_.make_transaction();
        auto ids = db->select(
            &database::project::id,
            where(c(&database::project::name) == project_name));
        if(!ids.size())
        {
            db->insert(
                    database::project{
                            -1,
                            project_name,
                            false});
            db.commit();
        }
    }
    auto [it, ok] = projects_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(project_name),
            std::forward_as_tuple(child_node));
    return {&(it->second), {it}};
}

void project_list::remove_child(std::any it)
{
    auto project_it = std::any_cast<
            std::map<std::string, project>::iterator>(it);
    projects_.erase(project_it);
}

immutable_container_proxy<
        std::map<std::string, project>> project_list::get_projects()
{
    return {projects_};
}

const std::map<std::string, project>& project_list::get_projects() const
{
    return projects_;
}

} // namespace cis
