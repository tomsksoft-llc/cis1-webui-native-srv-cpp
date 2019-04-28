#include "fs_mapper.h"

#include <string>
#include <utility>

#include <boost/regex.hpp>

namespace cis
{

fs_mapper::fs_mapper(fs_mapper&& other) noexcept
    : self_(other.self_)
    , type_(other.type_)
{
    other.detached_ = true;
}

fs_mapper::~fs_mapper()
{
    if(detached_)
    {
        return;
    }
    destroy();
}

void fs_mapper::init(fs_cache_node<fs_mapper>* self)
{
    if(self_ != nullptr)
    {
        destroy();
    }
    self_ = self;
    switch(self_->nesting_level_)
    {
        case 0:
        {
            type_ = cis_obj_type::project_list;
            break;
        }
        case 1:
        {
            if(self_->dir_entry_.is_directory())
            {
                std::tie(context_, context_it_) =
                        parent().context_->make_child(self_);
                type_ = cis_obj_type::project;
            }
            break;
        }
        case 2:
        {
            auto& childs = self_->childs_;
            if(auto it = childs.find(
                        "job.conf",
                        fs_cache<fs_mapper>::comparator{});
                    it != childs.end() && it->dir_entry_.is_regular_file())
            {
                std::tie(context_, context_it_) =
                        parent().context_->make_child(self_);
                type_ = cis_obj_type::job;
            }
            break;
        }
        case 3:
        {
            static auto is_build = [](const std::string& dir_name)
            {
                static const boost::regex build_mask("^\\d{6}$");
                return boost::regex_match(dir_name, build_mask);
            };
            if(     parent().type_ == cis_obj_type::job
                 && self_->dir_entry_.is_directory()
                 && is_build(self_->filename()))
            {
                std::tie(context_, context_it_) =
                        parent().context_->make_child(self_);
                type_ = cis_obj_type::build;
            }
            else if(parent().type_ == cis_obj_type::job
                 && self_->dir_entry_.is_regular_file()
                 && self_->filename() == "job.params")
            {
                static_cast<job_entry_interface*>(
                        parent().context_)->add_params(self_);
                type_ = cis_obj_type::job_params;
            }
            break;
        }
        case 4:
        {
            if(    parent().parent().type_ == cis_obj_type::job
                && parent().type_ == cis_obj_type::build
                && self_->filename() == "exitcode.txt")
            {
                static_cast<build_entry_interface*>(
                        parent().context_)->add_exitcode(self_);
                type_ = cis_obj_type::build_exitcode;
            }
            else if(
                   parent().parent().type_ == cis_obj_type::job
                && parent().type_ == cis_obj_type::build
                && self_->filename() == "output.txt")
            {
                static_cast<build_entry_interface*>(
                        parent().context_)->add_output(self_);
                type_ = cis_obj_type::build_output;
            }
        }
        default:
        {
            //noop
        }
    }
}

void fs_mapper::update_self_ptr(fs_cache_node<fs_mapper>* self)
{
    self_ = self;
    if(context_ != nullptr)
    {
        context_->set_fs_node(self_);
    }
}

void fs_mapper::set_context(cis_entry_interface* context)
{
    context_ = context;
    context_->set_fs_node(self_);
}

void fs_mapper::detach()
{
    detached_ = true;
}

void fs_mapper::destroy()
{
    switch(type_)
    {
        case cis_obj_type::project_list:
        {
            break;
        }
        case cis_obj_type::project:
        {
            parent().context_->remove_child(context_it_);
            break;
        }
        case cis_obj_type::job:
        {
            parent().context_->remove_child(context_it_);
            break;
        }
        case cis_obj_type::job_conf:
        {
            parent().parent().context_->remove_child(parent().context_it_);
            parent().type_ = cis_obj_type::regular_entry;
            break;
        }
        case cis_obj_type::job_params:
        {
            if(parent().type_ == cis_obj_type::job)
            {
                static_cast<job_entry_interface*>(
                        parent().context_)->remove_params();
            }
            break;
        }
        case cis_obj_type::build:
        {
            if(parent().type_ == cis_obj_type::job)
            {
                parent().context_->remove_child(context_it_);
            }
            break;
        }
        case cis_obj_type::build_exitcode:
        {
            if(    parent().parent().type_ == cis_obj_type::job
                && parent().type_ == cis_obj_type::build)
            {
                static_cast<build_entry_interface*>(
                        parent().context_)->remove_exitcode();
            }
            break;
        }
        case cis_obj_type::build_output:
        {
            if(    parent().parent().type_ == cis_obj_type::job
                && parent().type_ == cis_obj_type::build)
            {
                static_cast<build_entry_interface*>(
                        parent().context_)->remove_output();
            }
            break;
        }
        default:
        {
            //noop
        }
    }
}

fs_mapper& fs_mapper::parent()
{
    return self_->parent_->data_;
}

} // namespace cis
