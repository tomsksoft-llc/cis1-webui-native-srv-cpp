#include "cis/session_manager.h"

namespace cis
{

session_manager::session_manager(boost::asio::io_context& ctx)
    : ctx_(ctx)
{}

std::shared_ptr<session> session_manager::connect(
        const std::string& session_id)
{
    if(auto it = sessions_.find(session_id); it != sessions_.end())
    {
        auto session = it->second.lock();

        session->establish();

        return session;
    }
    else
    {
        auto s = std::shared_ptr<session>(
                        new session(ctx_),
                        [&, session_id](session* s)
                        {
                            delete s;

                            sessions_.erase(session_id);
                        });

        auto [res, success] = sessions_.emplace(
                session_id,
                s);

        s->establish();

        assert(success);

        return s;
    }
}

void session_manager::subscribe(
        const std::string& session_id,
        uint64_t ws_session_id,
        std::shared_ptr<subscriber_interface> subscriber)
{
    if(auto it = sessions_.find(session_id); it != sessions_.end())
    {
        if(auto session = it->second.lock(); session)
        {
            session->subscribe(ws_session_id, subscriber);
            subscriber->set_session(session);
        }
    }
    else
    {
        auto s = std::shared_ptr<session>(
                        new session(ctx_),
                        [&, session_id](session* s)
                        {
                            delete s;

                            sessions_.erase(session_id);
                        });

        auto [res, success] = sessions_.emplace(
                session_id,
                s);

        s->subscribe(ws_session_id, subscriber);
        subscriber->set_session(s);

        assert(success);
    }
}

std::shared_ptr<subscriber_interface> session_manager::get_subscriber(
        const std::string& session_id,
        uint64_t ws_session_id)
{
    if(auto it = sessions_.find(session_id); it != sessions_.end())
    {
        if(auto session = it->second.lock(); session)
        {
            return session->get_subscriber(ws_session_id);
        }
    }

    return nullptr;
}

void session_manager::finish_session(const std::string& session_id)
{
    if(auto it = sessions_.find(session_id); it != sessions_.end())
    {
        if(auto session = it->second.lock(); session)
        {
            return session->finish();
        }
    }
}

} // namespace cis
