#pragma once

#include <memory>
#include <functional>
#include <deque>

#include <boost/beast.hpp>
#include <boost/asio.hpp>

#include "fail.h"
#include "beast_ext/sink_body.h"

namespace net
{

class http_handler_interface;

class http_session
    : public std::enable_shared_from_this<http_session>
{
    class http_session_queue;
    class http_request_reader;
public:
    using request_reader = http_request_reader;
    using queue = http_session_queue;
    // Take ownership of the socket
    explicit http_session(
        boost::asio::ip::tcp::socket socket,
        std::shared_ptr<http_handler_interface const> app);

    // Start the asynchronous operation
    void run();

    void do_read_header();

    // Called when the timer expires.
    void on_timer(boost::beast::error_code ec);

    void on_read_header(boost::beast::error_code ec);

    template <class Body>
    void on_read_body(
            std::function<void(
                    boost::beast::http::request<Body>&&,
                    http_session_queue&)> cb,
            boost::beast::error_code ec,
            std::size_t bytes_transferred)
    {
        // Happens when the timer closes the socket
        if(ec == boost::asio::error::operation_aborted)
        {
            return;
        }

        // This means they closed the connection
        if(ec == boost::beast::http::error::end_of_stream)
        {
            return do_close();
        }

        if(ec)
        {
            return fail(ec, "read");
        }

        if(cb)
        {
            cb(request_reader_.get_parser<Body>().release(), queue_);
        }

        // If we aren't at the queue limit, try to pipeline another request
        if(!queue_.is_full())
        {
            do_read_header();
        }
    }

    void on_write(boost::beast::error_code ec, bool close);

    void do_close();
private:
    class http_session_queue
    {
    public:
        explicit http_session_queue(http_session& self);

        // Returns `true` if we have reached the queue limit
        bool is_full() const;

        // Called when a message finishes sending
        // Returns `true` if the caller should initiate a read
        bool on_write();

        template<bool isRequest, class Body, class Fields>
        void send(boost::beast::http::message<isRequest, Body, Fields>&& msg)
        {
            // This holds a work item
            struct work_impl : work
            {
                http_session& self_;
                boost::beast::http::message<isRequest, Body, Fields> msg_;

                work_impl(
                        http_session& self,
                        boost::beast::http::message<isRequest, Body, Fields>&& msg)
                        : self_(self)
                        , msg_(std::move(msg))
                {
                }

                void operator()()
                {
                    boost::beast::http::async_write(
                            self_.socket_,
                            msg_,
                            boost::asio::bind_executor(
                                    self_.strand_,
                                    std::bind(
                                            &http_session::on_write,
                                            self_.shared_from_this(),
                                            std::placeholders::_1,
                                            msg_.need_eof())));
                }
            };

            // Allocate and store the work
            items_.push_back(
                    boost::make_unique<work_impl>(self_, std::move(msg)));

            // If there was no previous work, start this one
            if(items_.size() == 1)
            {
                (*items_.front())();
            }
        }
    private:
        enum
        {
            // Maximum number of responses we will queue
            limit = 8
        };

        // The type-erased, saved work item
        struct work
        {
            virtual ~work() = default;
            virtual void operator()() = 0;
        };

        http_session& self_;
        std::deque<std::unique_ptr<work>> items_;
    };
    class http_request_reader
    {
    public:
        explicit http_request_reader(http_session& self)
            : self_(self)
        {}

        template<class Body>
        void async_read_body(
                std::function<void(boost::beast::http::request<Body>&)> pre,
                std::function<void(
                        boost::beast::http::request<Body>&&,
                        http_session_queue&)> cb)
        {
            upgrade_parser<Body>();

            pre(get_parser<Body>().get());

            boost::beast::http::async_read(
                    self_.socket_,
                    self_.buffer_,
                    get_parser<Body>(),
                    boost::asio::bind_executor(
                            self_.strand_,
                            std::bind(
                                    &http_session::on_read_body<Body>,
                                    self_.shared_from_this(),
                                    cb,
                                    std::placeholders::_1,
                                    std::placeholders::_2)));
            done_ = true;
        }

        void done();

    private:
        struct parser_wrapper
        {
            virtual ~parser_wrapper() = default;
            virtual const std::type_info& type_info() const = 0;
        };

        template <class Body>
        struct parser_wrapper_impl : parser_wrapper
        {
            boost::beast::http::request_parser<Body> parser;
            parser_wrapper_impl() = default;
            parser_wrapper_impl(
                    boost::beast::http::request_parser<boost::beast::http::empty_body>&& other)
                : parser(std::move(other))
            {}
            const std::type_info& type_info() const
            {
                return typeid(parser);
            }
        };


        friend class http_session;
        http_session& self_;
        std::unique_ptr<parser_wrapper> parser_;
        bool done_ = false;


        template<class Body>
        boost::beast::http::request_parser<Body>& get_parser()
        {
            if(typeid(boost::beast::http::request_parser<Body>) != parser_->type_info())
            {
                throw std::runtime_error("Bad parser cast");
            }
            return static_cast<parser_wrapper_impl<Body>&>(*parser_).parser;
        }

        boost::beast::http::request_parser<boost::beast::http::empty_body>& get_header_parser();

        void prepare();
        template<class Body>
        void upgrade_parser()
        {
            parser_ = std::make_unique<parser_wrapper_impl<Body>>(
                    std::move(get_header_parser()));
        }
    };

    boost::asio::ip::tcp::socket socket_;
    boost::asio::strand<
        boost::asio::io_context::executor_type> strand_;
    boost::asio::steady_timer timer_;
    boost::beast::flat_buffer buffer_;
    std::shared_ptr<http_handler_interface const> app_;
    http_session_queue queue_;
    http_request_reader request_reader_;
    friend class http_session_queue;
    friend class http_request_reader;
};

template <>
void http_session::http_request_reader::upgrade_parser<boost::beast::http::empty_body>();

} // namespace net
