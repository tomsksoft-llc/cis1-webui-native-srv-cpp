/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

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

fs_iterator& fs_entry_ref::iterator()
{
    return it_;
}

const std::filesystem::directory_entry& fs_entry_ref::dir_entry() const
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

std::map<std::string, std::string> load_job_conf(
        const std::filesystem::path& path)
{
    std::map<std::string, std::string> conf;

    std::ifstream conf_file(path);
    while(!conf_file.eof())
    {
        std::string key;
        std::string val;

        std::getline(conf_file, key, '=');

        if(key.empty() && conf_file.good() && !conf_file.eof())
        {
            conf.clear();

            return conf;
        }
        else if(conf_file.eof())
        {
            break;
        }

        std::getline(conf_file, val, '\n');

        val.erase(
                std::find_if(
                        val.rbegin(),
                        val.rend(),
                        [](int ch)
                        {
                            return !std::isspace(ch);
                        }).base(),
                val.end());

        conf[key] = val;
    }

    if(conf.count("script") != 1
    || conf.count("keep_last_success_builds") != 1
    || conf.count("keep_last_break_builds") != 1)
    {
        conf.clear();

        return conf;
    }

    try
    {
        stoul(conf["keep_last_success_builds"]);
        stoul(conf["keep_last_break_builds"]);
    }
    catch(...)
    {
        conf.clear();

        return conf;
    }

    return conf;
}

bool job::is_entry(fs_iterator& it)
{
    if(!it->is_directory())
    {
        return false;
    }

    auto job_conf_entry = it.find("job.conf");

    if(job_conf_entry == it.end())
    {
        return false;
    }

    return !load_job_conf(job_conf_entry->path()).empty();
}

job::job(const fs_iterator& it)
    : fs_entry_ref(it)
{
    auto job_conf = it_.find("job.conf");

    std::map<std::string, std::string> conf = load_job_conf(job_conf->path());

    config_.script = conf["script"];
    config_.keep_successful_builds = stoul(conf["keep_last_success_builds"]);
    config_.keep_broken_builds = stoul(conf["keep_last_break_builds"]);

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

std::shared_ptr<fs_entry_interface> job::get_script_entry()
{
    if(auto it = it_.find(config_.script); it != it_.end())
    {
        return std::make_shared<fs_entry_ref>(it);
    }

    return nullptr;
}

std::shared_ptr<fs_entry_interface> job::get_params_entry()
{
    if(auto it = it_.find("job.params"); it != it_.end())
    {
        return std::make_shared<fs_entry_ref>(it);
    }

    return nullptr;
}

std::shared_ptr<fs_entry_interface> job::get_conf_entry()
{
    if(auto it = it_.find("job.conf"); it != it_.end())
    {
        return std::make_shared<fs_entry_ref>(it);
    }

    return nullptr;
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

    if(auto session_id_it = it_.find("session_id.txt");
            session_id_it != it_.end()
            && session_id_it->is_regular_file())
    {
        std::ifstream session_id_file(session_id_it->path());
        std::string session_id;
        session_id_file >> session_id;

        info_.session_id = session_id;

        if(session_id.length() > 19)
        {
            info_.date = session_id.substr(0, 19);
        }
    }

    if(auto exit_message_it = it_.find("exitmessage.txt");
            exit_message_it != it_.end()
            && exit_message_it->is_regular_file())
    {
        std::ifstream exit_message_file(exit_message_it->path());
        std::string exit_message;
        exit_message_file.seekg(0, std::ios::end);
        exit_message.reserve(exit_message_file.tellg());
        exit_message_file.seekg(0, std::ios::beg);

        exit_message.assign(
                (std::istreambuf_iterator<char>(exit_message_file)),
                std::istreambuf_iterator<char>());

        info_.exit_message = exit_message;
    }
}

const build::info& build::get_info()
{
    return info_;
}

} // namespace cis
