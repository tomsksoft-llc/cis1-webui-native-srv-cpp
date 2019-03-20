#pragma once

#include <filesystem>
#include <vector>

#include <boost/process.hpp>

class child_process
    : public std::enable_shared_from_this<child_process>
{
public:
    child_process(
            boost::asio::io_context& ctx,
            const boost::process::environment& env);
    ~child_process();
    void run(
            const std::string& programm,
            std::vector<std::string> args,
            const std::function<void(
                int,
                std::vector<char>&,
                const std::error_code&)>& cb);
private:
    boost::asio::io_context& ctx_;
    boost::process::environment env_;
    std::filesystem::path start_dir_;
    std::vector<char> buffer_;
    boost::process::child *proc_ = nullptr;    
};
