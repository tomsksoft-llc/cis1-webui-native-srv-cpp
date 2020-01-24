#pragma once

#include <memory>

#include <cis1_cwu_protocol/protocol.h>

namespace cis
{

class session_interface;

struct subscriber_interface
{
    virtual ~subscriber_interface() = default;
    virtual void log_entry(const cis1::cwu::log_entry& dto) = 0;
    virtual void session_not_established() = 0;
    virtual void session_closed() = 0;
    virtual void set_session(std::shared_ptr<session_interface> set_session) = 0;
    virtual void unsubscribe() = 0;
};

struct session_interface
{
    virtual ~session_interface() = default;

    virtual void establish() = 0;

    virtual void log(const cis1::cwu::log_entry& dto) = 0;

    virtual void subscribe(
            uint64_t ws_session_id,
            std::shared_ptr<subscriber_interface> subscriber) = 0;

    virtual std::shared_ptr<subscriber_interface> get_subscriber(
            uint64_t ws_session_id) = 0;

    virtual void finish() = 0;
};

} // namespace cis
