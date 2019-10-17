#include "child_process.h"

#include <iostream>

#include "cis/dirs.h"

child_process::child_process(
        boost::asio::io_context& ctx,
        const boost::process::environment& env,
        const std::filesystem::path& start_dir,
        const std::string& program)
    : ctx_(ctx)
    , env_(env)
    , start_dir_(start_dir)
    , program_(program)
    , proc_(nullptr)
    , exit_code_(std::nullopt)
    , std_in_(std::make_shared<boost::process::async_pipe>(ctx))
    , std_out_(std::make_shared<boost::process::async_pipe>(ctx))
    , std_err_(std::make_shared<boost::process::async_pipe>(ctx))
{}

void child_process::run(
        std::vector<std::string> args,
        const std::shared_ptr<process_io_handler_interface>& io,
        const std::shared_ptr<task_callback_interface>& callbacks)
{
    namespace bp = boost::process;

    if(io)
    {
        io->accept_std_in(std_in_);
        io->accept_std_out(std_out_);
        io->accept_std_err(std_err_);
    }

    try
    {
        proc_ = std::make_unique<bp::child>(
                program_,
                env_,
                ctx_,
                bp::std_in < *std_in_,
                bp::std_out > *std_out_,
                bp::std_err > *std_err_,
                bp::start_dir = start_dir_.c_str(),
                bp::args = args,
                bp::on_exit =
                        [
                        &,
                        self = shared_from_this(),
                        io = std::move(io),
                        callbacks = std::move(callbacks)
                        ](
                                int exit_code,
                                const std::error_code& ec) mutable
                        {
                            exit_code_ = exit_code;
                            finished_ = true;

                            if(!callbacks)
                            {
                                return;
                            }

                            if(!ec)
                            {
                                callbacks->on_success();
                            }
                            else
                            {
                                callbacks->on_error();
                            }
                        });
    }
    catch(const boost::process::process_error& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void child_process::cancel()
{
    //TODO
}

bool child_process::finished()
{
    return finished_;
}

std::optional<int> child_process::exit_code()
{
    return exit_code_;
}

std::vector<char> make_interactive_args_buffer(
        const std::vector<std::string>& args)
{
    std::vector<char> buffer;

    for(auto& param : args)
    {
        std::copy(
                param.begin(),
                param.end(),
                std::back_inserter(buffer));

        buffer.push_back('\n');
    }

    return buffer;
}
