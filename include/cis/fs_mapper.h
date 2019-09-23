#pragma once

#include <any>

#include "fs_cache.h"

namespace cis
{

struct fs_mapper;

struct cis_entry_interface
{
    virtual ~cis_entry_interface() = default;
    virtual void set_fs_node(
            fs_cache_node<fs_mapper>* fs_node) = 0;
    virtual std::pair<cis_entry_interface*, std::any> make_child(
            fs_cache_node<fs_mapper>* child_node) = 0;
    virtual void remove_child(
            std::any it) = 0;
};

struct job_entry_interface
    : public cis_entry_interface
{
    virtual void add_params(fs_cache_node<fs_mapper>* fs_node) = 0;
    virtual void remove_params() = 0;
};

struct build_entry_interface
    : public cis_entry_interface
{
    virtual void add_output(fs_cache_node<fs_mapper>* fs_node) = 0;
    virtual void remove_output() = 0;
    virtual void add_exitcode(fs_cache_node<fs_mapper>* fs_node) = 0;
    virtual void remove_exitcode() = 0;
};

class fs_mapper
{
public:
    enum class cis_obj_type
    {
        project_list,
        project,
        job,
        job_conf,
        job_params,
        build,
        build_exitcode,
        build_output,
        regular_entry
    };

    fs_mapper() = default;
    fs_mapper(const fs_mapper& other) = delete;
    fs_mapper(fs_mapper&& other) noexcept;

    fs_mapper& operator=(const fs_mapper& other) = delete;
    fs_mapper& operator=(fs_mapper&& other) = delete;

    ~fs_mapper();

    void init(fs_cache_node<fs_mapper>* self);

    void update_self_ptr(fs_cache_node<fs_mapper>* self);
    void set_context(cis_entry_interface* context);
    void detach();

    void destroy();
private:
    fs_mapper& parent();
    fs_cache_node<fs_mapper>* self_ = nullptr;
    cis_entry_interface* context_ = nullptr;
    std::any context_it_;
    cis_obj_type type_ = cis_obj_type::regular_entry;
    bool detached_ = false;
};

} // namespace cis
