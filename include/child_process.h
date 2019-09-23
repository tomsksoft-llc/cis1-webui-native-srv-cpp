#pragma once

#include <optional>
#include <filesystem>
#include <vector>
#include <memory>

#include <boost/process.hpp>

class child_process
    : public std::enable_shared_from_this<child_process>
{
public:
    using on_exit_cb_t = std::function<void(
                    int,
                    const std::vector<char>&)>;

    child_process(
            boost::asio::io_context& ctx,
            const boost::process::environment& env);

    void set_interactive_params(
            const std::vector<std::string>& params);

    void run(
            const std::string& programm,
            std::vector<std::string> args,
            const on_exit_cb_t& cb,
            bool ignore_output = false);

private:
    boost::asio::io_context& ctx_;
    boost::process::environment env_;
    std::filesystem::path start_dir_;
    boost::process::async_pipe pipe_;
    std::vector<char> buffer_;
    std::vector<std::string> interactive_params_;
    std::unique_ptr<boost::process::child> proc_;
    on_exit_cb_t cb_;
    const size_t read_size = 512;
    std::optional<int> exit_code_;

    void do_read(
            std::shared_ptr<child_process> self,
            size_t offset = 0);

    void on_done();
};
