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

#include "session_interface.h"

namespace cis
{

class session
    : public session_interface
    , public std::enable_shared_from_this<session>
{
public:
    session(boost::asio::io_context& ctx);

    void init_timer();

    void establish() override;

    void log(const cis1::cwu::log_entry& dto) override;

    void subscribe(
            uint64_t ws_session_id,
            std::shared_ptr<subscriber_interface> subscriber) override;

    std::shared_ptr<subscriber_interface> get_subscriber(
            uint64_t ws_session_id) override;

    void finish() override;

private:
    enum class state
    {
        pending,
        not_established,
        in_progress,
        finished,
    };
    state state_ = state::pending;
    std::vector<cis1::cwu::log_entry> received_logs_;
    std::map<uint64_t, std::weak_ptr<subscriber_interface>> subscribers_;
    boost::asio::steady_timer timer_;

    void on_timer(const boost::system::error_code& ec);
};

} // namespace cis
