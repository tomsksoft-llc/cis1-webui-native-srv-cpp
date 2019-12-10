/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <vector>
#include <map>

#include <boost/asio.hpp>

#include <cis1_cwu_protocol/protocol.h>

namespace cis
{

class session;

struct subscriber_interface
{
    virtual ~subscriber_interface() = default;
    virtual void log_entry(const cis1::cwu::log_entry& dto) = 0;
    virtual void session_not_established() = 0;
    virtual void session_closed() = 0;
    virtual void set_session(std::shared_ptr<session> set_session) = 0;
    virtual void unsubscribe() = 0;
};

class session
{
public:
    session(boost::asio::io_context& ctx);

    enum class state
    {
        pending,
        not_established,
        in_progress,
        finished,
    };

    void establish();

    void log(const cis1::cwu::log_entry& dto);

    void subscribe(
            uint64_t ws_session_id,
            std::shared_ptr<subscriber_interface> subscriber);

    std::shared_ptr<subscriber_interface> get_subscriber(uint64_t ws_session_id);

    void finish();

private:
    state state_ = state::pending;
    std::vector<cis1::cwu::log_entry> received_logs_;
    std::map<uint64_t, std::weak_ptr<subscriber_interface>> subscribers_;
    boost::asio::steady_timer timer_;

    void on_timer(const boost::system::error_code& ec);
};

} // namespace cis
