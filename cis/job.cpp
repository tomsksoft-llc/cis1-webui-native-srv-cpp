#include "job.h"

#include <filesystem>
#include <iostream>

#include <boost/process.hpp>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "child_process.h"
#include "dirs.h"
#include "file_util.h"

namespace cis
{

void run_job(
        boost::asio::io_context& ctx,
        const std::string& project,
        const std::string& name)
{
    auto env = boost::this_process::environment();
    env["cis_base_dir"] = cis::get_root_dir();
    auto cp = std::make_shared<child_process>(ctx, env);
    cp->run(
            "sh",
            {"startjob", path_cat(project, "/" + name)},
            [project, name](int exit, std::vector<char>& buffer, const std::error_code& ec)
            {
                std::cout << "job " + project
                        + "/" + name + " finished" << std::endl;
                if(!ec)
                {
                    std::cout << "process exited with " << exit << std::endl;
                    std::cout << "std_out contain:" << std::endl;
                    std::cout.write(buffer.data(), buffer.size());
                }
                else
                {
                    std::cout << "error" << std::endl;
                }
                //TODO update builds (emit cis_updated or something)
            });
}

void rename_job(
        const std::string& project,
        const std::string& old_name,
        const std::string& new_name,
        std::error_code& ec)
{
    auto project_path = std::filesystem::path{cis::get_root_dir()} / cis::projects / project;
    std::filesystem::rename(project_path /old_name, project_path / new_name, ec);
}

} // namespace cis
