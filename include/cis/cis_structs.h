#pragma once

#include <map>
#include <vector>
#include <optional>
#include <string>

#include "cis_structs_interface.h"

namespace cis
{

class fs_entry_ref
    : virtual public fs_entry_interface
{
public:
    fs_entry_ref(const fs_iterator& it);

    fs_iterator get_files() override;

    void invalidate() override;

    const std::filesystem::directory_entry& dir_entry() const override;

protected:
    fs_iterator it_;
};

class project
    : public project_interface
    , public fs_entry_ref
{
public:
    static bool is_entry(fs_iterator& it);

    project(const fs_iterator& it);

    std::shared_ptr<job_interface> get_job_info(
            const std::string& job_name) override;

    job_list_t get_job_list() override;
};

class job
    : public job_interface
    , public fs_entry_ref
{
public:
    static bool is_entry(fs_iterator& it);

    job(const fs_iterator& it);

    build_list_t get_build_list() override;

    std::shared_ptr<build_interface> get_build_info(
            const std::string& build_name) override;

    const std::vector<param>& get_params() override;

private:
    std::vector<param> params_;
};

class build
    : public build_interface
    , public fs_entry_ref
{
public:
    static bool is_entry(fs_iterator& it);

    build(const fs_iterator& it);

    const info& get_info() override;

private:
    info info_;
};

} // namespace cis
