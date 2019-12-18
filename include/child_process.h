/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <optional>
#include <filesystem>
#include <vector>
#include <memory>
#include <iostream>

#include <boost/process.hpp>
#include <boost/asio.hpp>

struct task_callback_interface
{
    virtual ~task_callback_interface() = default;

    virtual void on_success() = 0;
    virtual void on_error() = 0;
};

class task_callback
    : public task_callback_interface
{
public:
    task_callback(
            std::function<void()> on_success,
            std::function<void()> on_error)
        : on_success_(on_success)
        , on_error_(on_error)
    {}

    void on_success() override
    {
        on_success_();
    }

    void on_error() override
    {
        on_error_();
    }

private:
    std::function<void()> on_success_;
    std::function<void()> on_error_;
};

struct process_io_handler_interface
{
    virtual ~process_io_handler_interface() = default;

    virtual void accept_std_in(std::shared_ptr<boost::process::async_pipe> in) = 0;
    virtual void accept_std_out(std::shared_ptr<boost::process::async_pipe> out) = 0;
    virtual void accept_std_err(std::shared_ptr<boost::process::async_pipe> err) = 0;
};

class process_io_handler
    : public process_io_handler_interface
{
public:
    process_io_handler(
            std::function<void(std::shared_ptr<boost::process::async_pipe>)> in_handler,
            std::function<void(std::shared_ptr<boost::process::async_pipe>)> out_handler,
            std::function<void(std::shared_ptr<boost::process::async_pipe>)> err_handler)
        : in_handler_(in_handler)
        , out_handler_(out_handler)
        , err_handler_(err_handler)
    {}

    void accept_std_in(std::shared_ptr<boost::process::async_pipe> in) override
    {
        in_handler_(in);
    }

    void accept_std_out(std::shared_ptr<boost::process::async_pipe> out) override
    {
        out_handler_(out);
    }

    void accept_std_err(std::shared_ptr<boost::process::async_pipe> err) override
    {
        err_handler_(err);
    }

private:
    std::function<void(std::shared_ptr<boost::process::async_pipe>)> in_handler_;
    std::function<void(std::shared_ptr<boost::process::async_pipe>)> out_handler_;
    std::function<void(std::shared_ptr<boost::process::async_pipe>)> err_handler_;
};

class async_buffer_writer
    : public std::enable_shared_from_this<async_buffer_writer>
{
public:
    async_buffer_writer(std::vector<char>&& buffer)
        : buffer_(std::move(buffer))
    {}

    void accept_pipe(std::shared_ptr<boost::process::async_pipe> pipe)
    {
        write_buffer(pipe);
    }
private:
    std::vector<char> buffer_;
    size_t total_bytes_transferred_ = 0;

    void write_buffer(std::shared_ptr<boost::process::async_pipe> pipe)
    {
        boost::asio::async_write(
                *pipe,
                boost::asio::buffer(
                        buffer_.data() + total_bytes_transferred_,
                        buffer_.size() - total_bytes_transferred_),
                [&, pipe, self = shared_from_this()](
                        const boost::system::error_code& ec,
                        std::size_t bytes_transferred)
                {
                    if(ec)
                    {
                        return;
                    }

                    total_bytes_transferred_ += bytes_transferred;

                    if(total_bytes_transferred_ < buffer_.size())
                    {
                        write_buffer(pipe);
                    }
                    else
                    {
                        pipe->close();
                    }
                });
    }
};

std::vector<char> make_interactive_args_buffer(
        const std::vector<std::string>& args);

class buffer_reader
    : public std::enable_shared_from_this<buffer_reader>
{
public:
    buffer_reader(const std::function<void(const std::vector<char>&)>& cb)
        : cb_(cb)
    {}

    void accept_pipe(std::shared_ptr<boost::process::async_pipe> pipe)
    {
        read_all(pipe);
    }

private:
    boost::asio::streambuf buffer_;
    std::function<void(const std::vector<char>&)> cb_;

    void read_all(std::shared_ptr<boost::process::async_pipe> pipe)
    {
        boost::asio::async_read(
                *pipe,
                buffer_,
                [&, pipe, self = shared_from_this()](
                        const boost::system::error_code& ec,
                        std::size_t bytes_transferred)
                {
                    if(ec && ec != boost::asio::error::eof)
                    {
                        return;
                    }
                    else
                    {
                        finish();
                        return;
                    }

                    if(pipe->is_open())
                    {
                        read_all(pipe);
                    }
                    else
                    {
                        finish();
                    }
                });
    }

    void finish()
    {
        std::vector<char> target(buffer_.size());
        buffer_copy(boost::asio::buffer(target), buffer_.data());
        cb_(target);
    }
};

class line_reader
    : public std::enable_shared_from_this<line_reader>
{
public:
    line_reader(const std::function<void(const std::string&)>& cb)
        : cb_(cb)
    {}

    void accept_pipe(std::shared_ptr<boost::process::async_pipe> pipe)
    {
        read_line(pipe);
    }

    const std::string& last_line()
    {
        return last_line_;
    }

private:
    boost::asio::streambuf buffer_;
    std::string last_line_;
    std::function<void(const std::string&)> cb_;

    void read_line(std::shared_ptr<boost::process::async_pipe> pipe)
    {
        boost::asio::async_read_until(
                *pipe,
                buffer_,
                '\n',
                [&, pipe, self = shared_from_this()](
                        const boost::system::error_code& ec,
                        std::size_t bytes_transferred)
                {
                    if(ec)
                    {
                        return;
                    }

                    std::istream stream(&buffer_);
                    last_line_.clear();
                    std::getline(stream, last_line_);
                    cb_(last_line_);

                    if(pipe->is_open())
                    {
                        read_line(pipe);
                    }
                });
    }
};

class child_process
    : public std::enable_shared_from_this<child_process>
{
public:
    child_process(
            boost::asio::io_context& ctx,
            const boost::process::environment& env,
            const std::filesystem::path& start_dir,
            const std::string& program);

    void run(
            std::vector<std::string> args,
            const std::shared_ptr<process_io_handler_interface>& io,
            const std::shared_ptr<task_callback_interface>& callbacks);

    void cancel();

    bool finished();

    std::optional<int> exit_code();

private:
    boost::asio::io_context& ctx_;
    boost::process::environment env_;
    std::filesystem::path start_dir_;
    std::string program_;
    std::unique_ptr<boost::process::child> proc_;
    std::optional<int> exit_code_;
    bool finished_ = false;
    std::shared_ptr<boost::process::async_pipe> std_in_;
    std::shared_ptr<boost::process::async_pipe> std_out_;
    std::shared_ptr<boost::process::async_pipe> std_err_;
};
