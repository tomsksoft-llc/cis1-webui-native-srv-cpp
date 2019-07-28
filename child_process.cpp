#include "child_process.h"

#include <iostream>

#include "cis/dirs.h"

child_process::child_process(
        boost::asio::io_context& ctx,
        const boost::process::environment& env)
    : ctx_(ctx)
    , env_(env)
    , start_dir_(std::filesystem::path{cis::get_root_dir()} / cis::core)
    , pipe_(ctx)
    , proc_(nullptr)
{}

void child_process::set_interactive_params(
        const std::vector<std::string>& params)
{
    interactive_params_ = params;
}

void child_process::run(
        const std::string& programm,
        std::vector<std::string> args,
        const on_exit_cb_t& cb)
{
    namespace bp = boost::process;

    for(auto& param : interactive_params_)
    {
        std::copy(
                param.begin(),
                param.end(),
                std::back_inserter(buffer_));
        buffer_.push_back('\n');
    }

    cb_ = cb;

    try
    {
        proc_ = std::make_unique<bp::child>(
                programm,
                env_,
                ctx_,
                bp::std_in < boost::asio::buffer(buffer_),
                bp::std_out > pipe_,
                bp::start_dir = start_dir_.c_str(),
                bp::args = args,
                bp::on_exit =
                        [&, self = shared_from_this()](
                                int exit_code,
                                const std::error_code& ec)
                        {
                            exit_code_ = exit_code;
                            do_read(ec, std::move(self));
                        });
    }
    catch(const boost::process::process_error& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void child_process::do_read(
        const std::error_code& ec,
        std::shared_ptr<child_process> self,
        size_t offset)
{
    //TODO handle errors
    buffer_.resize(offset + read_size);
    boost::asio::async_read(
            pipe_,
            boost::asio::buffer(buffer_.data(), buffer_.size()),
            [&, self = std::move(self)](
                const std::error_code& ec,
                size_t transferred)
            {
                if(transferred < read_size)
                {
                    buffer_.resize(buffer_.size() - (read_size - transferred));
                    on_done();
                }
                else
                {
                    do_read(ec, std::move(self), buffer_.size());
                }
            });
}

void child_process::on_done()
{
    if(cb_)
    {
        cb_(exit_code_.value(), buffer_);
    }
}
