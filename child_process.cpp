#include "child_process.h"

child_process::child_process(
        boost::asio::io_context& ctx,
        boost::process::environment env,
        std::filesystem::path start_dir)
    : ctx_(ctx)
    , env_(env)
    , start_dir_(start_dir)
{
    buffer_.resize(1024);
}

child_process::~child_process()
{
    if(proc_ != nullptr)
    {
        delete proc_;
    }
}

void child_process::run(
        const std::string& programm,
        const std::string& arg1,
        const std::string& arg2,
        std::function<void(int, std::vector<char>&, const std::error_code&)> cb)
{
    namespace bp = boost::process;
    auto self = shared_from_this();
    proc_ = new bp::child(
        bp::search_path(programm),
        env_,
        bp::start_dir = start_dir_.c_str(),
        arg1,
        arg2,
        ctx_,
        //strand_.context(), //TODO
        bp::std_out > boost::asio::buffer(buffer_),
        bp::on_exit = 
        [&, cb, self](int exit, const std::error_code& ec)
        {
            cb(exit, buffer_, ec);
        });
}
