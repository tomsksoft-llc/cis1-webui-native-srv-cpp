#pragma once

#include <deque>
#include <memory>

#include <boost/beast/http.hpp>

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>

class http_session;

class http_session_queue
{
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
public:
    explicit http_session_queue(http_session& self);

    // Returns `true` if we have reached the queue limit
    bool is_full() const;

    // Called when a message finishes sending
    // Returns `true` if the caller should initiate a read
    bool on_write();

    template<bool isRequest, class Body, class Fields>
    void send(http::message<isRequest, Body, Fields>&& msg);
};
