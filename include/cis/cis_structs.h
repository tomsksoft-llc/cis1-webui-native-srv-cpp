#pragma once

#include <map>
#include <vector>
#include <optional>
#include <string>

#include "fs_mapper.h"
#include "immutable_container_proxy.h"
#include "database.h"

namespace cis
{

struct build_info
{
    std::optional<int> status;
    std::optional<std::string> date;
};

struct build
    : public build_entry_interface
{
    build(fs_cache_node<fs_mapper>* fs_node);
    virtual void set_fs_node(
            fs_cache_node<fs_mapper>* fs_node) override;
    virtual std::pair<cis_entry_interface*, std::any> make_child(
            fs_cache_node<fs_mapper>* child_node) override;
    virtual void remove_child(std::any it) override;
    virtual void add_output(
            fs_cache_node<fs_mapper>* fs_node) override;
    virtual void remove_output() override;
    virtual void add_exitcode(
            fs_cache_node<fs_mapper>* fs_node) override;
    virtual void remove_exitcode() override;
    void erase();
    const build_info& get_info() const;
    immutable_container_proxy<fs_cache_node<fs_mapper>::tree_t> get_files();
    const fs_cache_node<fs_mapper>::tree_t& get_files() const;
private:
    build_info info_;
    fs_cache_node<fs_mapper>* fs_node_ = nullptr;
};

struct param
{
    explicit param(
            std::string param_name,
            std::string param_default_value);
    std::string name;
    std::string default_value;
};

struct job
    : public job_entry_interface
{
    job(fs_cache_node<fs_mapper>* fs_node);
    virtual void set_fs_node(
            fs_cache_node<fs_mapper>* fs_node) override;
    virtual std::pair<cis_entry_interface*, std::any> make_child(
            fs_cache_node<fs_mapper>* child_node) override;
    virtual void remove_child(std::any it) override;
    virtual void add_params(
            fs_cache_node<fs_mapper>* fs_node) override;
    virtual void remove_params() override;
    void erase();
    void refresh() const;
    immutable_container_proxy<std::map<std::string, build>> get_builds();
    const std::map<std::string, build>& get_builds() const;
    const std::vector<param>& get_params() const;
    immutable_container_proxy<fs_cache_node<fs_mapper>::tree_t> get_files();
    const fs_cache_node<fs_mapper>::tree_t& get_files() const;
private:
    std::map<std::string, build> builds_;
    std::vector<param> params_;
    fs_cache_node<fs_mapper>* fs_node_ = nullptr;
};

struct project
    : public cis_entry_interface
{
    project(fs_cache_node<fs_mapper>* fs_node);
    virtual void set_fs_node(
            fs_cache_node<fs_mapper>* fs_node) override;
    virtual std::pair<cis_entry_interface*, std::any> make_child(
            fs_cache_node<fs_mapper>* child_node) override;
    virtual void remove_child(std::any it) override;
    void erase();
    immutable_container_proxy<std::map<std::string, job>> get_jobs();
    const std::map<std::string, job>& get_jobs() const;
    immutable_container_proxy<
            typename fs_cache_node<fs_mapper>::tree_t> get_files();
    const fs_cache_node<fs_mapper>::tree_t& get_files() const;
private:
    std::map<std::string, job> jobs_;
    fs_cache_node<fs_mapper>* fs_node_ = nullptr;
};

struct project_list
    : public cis_entry_interface
{
    project_list(database::database_wrapper& db);
    virtual void set_fs_node(
            fs_cache_node<fs_mapper>* fs_node) override;
    virtual std::pair<cis_entry_interface*, std::any> make_child(
            fs_cache_node<fs_mapper>* child_node) override;
    virtual void remove_child(std::any it) override;
    immutable_container_proxy<std::map<std::string, project>> get_projects();
    const std::map<std::string, project>& get_projects() const;
    immutable_container_proxy<fs_cache_node<fs_mapper>::tree_t> get_files();
    const fs_cache_node<fs_mapper>::tree_t& get_files() const;
private:
    database::database_wrapper& db_;
    std::map<std::string, project> projects_;
    fs_cache_node<fs_mapper>* fs_node_ = nullptr;
};

} // namespace cis
