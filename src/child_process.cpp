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
{
}

void child_process::set_interactive_params(
        const std::vector<std::string>& params)
{
    interactive_params_ = params;
}

void child_process::run(
        const std::string& programm,
        std::vector<std::string> args,
        const on_exit_cb_t& cb,
        bool ignore_output)
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
                        [&, self = shared_from_this(), ignore_output](
                                int exit_code,
                                const std::error_code& ec) mutable
                        {
                            exit_code_ = exit_code;
                            buffer_.clear();
                            if(!ignore_output)
                            {
                                do_read(std::move(self));
                            }
                            else
                            {
                                on_done();
                            }
                        });
    }
    catch(const boost::process::process_error& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void child_process::do_read(
        std::shared_ptr<child_process> self,
        size_t offset)
{
    buffer_.resize(offset + read_size);
    boost::asio::async_read(
            pipe_,
            boost::asio::buffer(
                    buffer_.data() + offset,
                    buffer_.size() - offset),
            [&, self = std::move(self)](
                const boost::system::error_code& ec,
                size_t transferred) mutable
            {
                if(ec == boost::asio::error::eof)
                {
                    buffer_.resize(buffer_.size() - (read_size - transferred));
                    on_done();
                }
                else if(!ec)
                {
                    buffer_.resize(offset + transferred);
                    do_read(std::move(self), offset + transferred);
                }
                else
                {
                    //TODO handle errors
                }
            });
}

void child_process::on_done()
{
    if(cb_)
    {
        cb_(exit_code_.value(), buffer_);
        cb_ = [](auto&&...){}; // reset std::function
    }
}
