/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "cis/session.h"

namespace cis
{

session::session(boost::asio::io_context& ctx)
    : timer_(
            ctx,
            (std::chrono::steady_clock::time_point::max)())
{}

void session::init_timer()
{
    timer_.expires_after(std::chrono::seconds(5));
    timer_.async_wait(
            [&, self = shared_from_this()](
                    const boost::system::error_code& ec)
            {
                on_timer(ec);
            });
}

void session::establish()
{
    state_ = state::in_progress;
    timer_.cancel();
}

void session::log(const cis1::cwu::log_entry& dto)
{
    received_logs_.push_back(dto);

    for(auto& [key, subscriber] : subscribers_)
    {
        if(auto s = subscriber.lock(); s)
        {
            s->log_entry(dto);
        }
    }
}

void session::subscribe(
        uint64_t ws_session_id,
        std::shared_ptr<subscriber_interface> subscriber)
{
    if(state_ == state::not_established)
    {
        subscriber->session_not_established();

        return;
    }

    for(auto& log_entry : received_logs_)
    {
        subscriber->log_entry(log_entry);
    }

    subscribers_.emplace(ws_session_id, subscriber);

    if(state_ == state::finished)
    {
        subscriber->session_closed();
    }
}

std::shared_ptr<subscriber_interface> session::get_subscriber(
        uint64_t ws_session_id)
{
    if(auto it = subscribers_.find(ws_session_id); it != subscribers_.end())
    {
        return it->second.lock();
    }

    return nullptr;
}

void session::finish()
{
    state_ = state::finished;

    for(auto& [key, subscriber] : subscribers_)
    {
        if(auto s = subscriber.lock(); s)
        {
            s->session_closed();
        }
    }
}

void session::on_timer(const boost::system::error_code& ec)
{
    if(ec == boost::asio::error::operation_aborted)
    {
        return;
    }

    if(timer_.expiry() <= std::chrono::steady_clock::now()
    && state_ == state::pending)
    {
        state_ = state::not_established;
        for(auto& [key, subscriber] : subscribers_)
        {
            if(auto s = subscriber.lock(); s)
            {
                s->session_not_established();
            }
        }
    }
}

} // namespace cis
