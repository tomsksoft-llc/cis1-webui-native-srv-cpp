#include "cis/cis_structs.h"

#include <utility>
#include <fstream>
#include <regex>

namespace cis
{

fs_entry_ref::fs_entry_ref(const fs_iterator& it)
    : it_(it)
{}

fs_iterator fs_entry_ref::get_files()
{
    return it_;
}

const std::filesystem::directory_entry& fs_entry_ref::dir_entry()
{
    return *it_;
}

void fs_entry_ref::invalidate()
{
    it_.invalidate();
}

bool project::is_entry(fs_iterator& it)
{
    return it->is_directory();
}

project::project(const fs_iterator& it)
    : fs_entry_ref(it)
{}

std::shared_ptr<job_interface> project::get_job_info(
        const std::string& job_name)
{
    if(auto it = it_.find(job_name);
            it != it_.end() && job::is_entry(it))
    {
        return std::make_shared<job>(it);
    }
    
    return nullptr;
}

project::job_list_t project::get_job_list()
{
    project::job_list_t list;
    
    for(auto it = it_.begin(); it != it_.end(); ++it)
    {
        if(job::is_entry(it))
        {
            list.push_back(
                    static_cast<std::shared_ptr<job_interface>>(
                            std::make_shared<job>(it)));
        }
        else
        {
            list.push_back(std::make_shared<fs_entry_ref>(it));
        }
    }

    return list;
}

bool job::is_entry(fs_iterator& it)
{
    return it->is_directory() && (it.find("job.conf") != it.end());
}

job::job(const fs_iterator& it)
    : fs_entry_ref(it)
{
    if(auto params = it_.find("job.params");
            params != it_.end()
            && params->is_regular_file())
    {
        std::ifstream params_file(params->path());
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

            if(!param.empty())
            {
                params_.push_back(job::param{param, default_value});
            }
        }
    }
}

job::build_list_t job::get_build_list()
{
    job::build_list_t list;

    for(auto it = it_.begin(); it != it_.end(); ++it)
    {
        if(build::is_entry(it))
        {
            list.push_back(
                    static_cast<std::shared_ptr<build_interface>>(
                            std::make_shared<build>(it)));
        }
        else
        {
            list.push_back(std::make_shared<fs_entry_ref>(it));
        }
    }
    
    return list;
}

std::shared_ptr<build_interface> job::get_build_info(
        const std::string& build_name)
{
    if(auto it = it_.find(build_name);
            it != it_.end() && build::is_entry(it))
    {
        return std::make_shared<build>(it);
    }
    
    return nullptr;
}

const std::vector<job::param>& job::get_params()
{
    return params_;
}


bool build::is_entry(fs_iterator& it)
{
    static const auto is_build = [](const std::string& dir_name)
    {
        static const std::regex build_mask("^\\d{6}$");
        return std::regex_match(dir_name, build_mask);
    };

    return it->is_directory() && is_build(it->path().filename());
}

build::build(const fs_iterator& it)
    : fs_entry_ref(it)
{
    if(auto exit_code = it_.find("exitcode.txt");
            exit_code != it_.end()
            && exit_code->is_regular_file())
    {
        std::ifstream exit_code_file(exit_code->path());
        int exitcode;
        exit_code_file >> exitcode;
        info_.status = exitcode;
    }

    if(auto session_id = it_.find("session_id.txt");
            session_id != it_.end()
            && session_id->is_regular_file())
    {
        std::ifstream session_id_file(session_id->path());
        std::string date;
        session_id_file >> date;

        if(date.length() > 19)
        {
            info_.date = date.substr(0, 19);
        }
    }
}

const build::info& build::get_info()
{
    return info_;
}

} // namespace cis
