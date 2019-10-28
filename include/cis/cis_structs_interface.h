#pragma once

#include <variant>
#include <memory>
#include <vector>

#include "fs_cache.h"

struct project_interface;
struct job_interface;
struct build_interface;

struct fs_entry_interface
{
    virtual ~fs_entry_interface() = default;

    virtual fs_iterator get_files() = 0;

    virtual void invalidate() = 0;

    virtual const std::filesystem::directory_entry& dir_entry() const = 0;
};

struct project_interface
    : virtual public fs_entry_interface
{
    virtual ~project_interface() = default;

    using job_list_t =
        std::vector<
                std::variant<
                        std::shared_ptr<fs_entry_interface>,
                        std::shared_ptr<job_interface>>>;

    virtual std::shared_ptr<job_interface> get_job_info(
            const std::string& job_name) = 0;

    virtual job_list_t get_job_list() = 0;
};

struct job_interface
    : virtual public fs_entry_interface
{
    struct param
    {
        std::string name;
        std::string default_value;
    };

    virtual ~job_interface() = default;

    using build_list_t =
        std::vector<
                std::variant<
                        std::shared_ptr<fs_entry_interface>,
                        std::shared_ptr<build_interface>>>;

    virtual build_list_t get_build_list() = 0;

    virtual std::shared_ptr<build_interface> get_build_info(
            const std::string& build_name) = 0;

    virtual const std::vector<param>& get_params() = 0;
};

struct build_interface
    : virtual public fs_entry_interface
{
    struct info
    {
        std::optional<int> status;
        std::optional<std::string> date;
    };

    virtual ~build_interface() = default;

    virtual const info& get_info() = 0;
};
