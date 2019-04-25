#include "child_process.h"

#include <iostream>

#include "cis/dirs.h"

child_process::child_process(
        boost::asio::io_context& ctx,
        const boost::process::environment& env)
    : ctx_(ctx)
    , env_(env)
    , start_dir_(std::filesystem::path{cis::get_root_dir()} / cis::core)
{
    //buffer_.resize(1024);
}

child_process::~child_process()
{
    if(proc_ != nullptr)
    {
        delete proc_;
    }
}

void child_process::set_interactive_params(
        const std::vector<std::string>& params)
{
    interactive_params_ = params;
}

void child_process::run(
        const std::string& programm,
        std::vector<std::string> args,
        const std::function<void(
                int,
                std::vector<char>&,
                const std::error_code&)>& cb)
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
    try
    {
        proc_ = new bp::child(
                programm,
                env_,
                ctx_,
                bp::std_in < boost::asio::buffer(buffer_),
                //bp::std_out > boost::asio::buffer(buffer_),
                bp::start_dir = start_dir_.c_str(),
                bp::args = args,
                bp::on_exit =
                        [&, cb, self = shared_from_this()](
                                int exit,
                                const std::error_code& ec)
                        {
                            //TODO run in strand_.context();
                            //cb(exit, buffer_, ec);
                        });
    }
    catch(const boost::process::process_error& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
