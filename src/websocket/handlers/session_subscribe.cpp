/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/session_subscribe.h"

#include <regex>

#include "websocket/dto/cis_session_log_entry.h"
#include "websocket/dto/cis_session_not_established.h"
#include "websocket/dto/cis_session_closed.h"

namespace websocket
{

namespace handlers
{

const char* const message_regex_expr =
        R"(\s*action=\")"
        R"(([a-z_]+))"
        R"(\"\s*)"
        R"((.*))";

const std::regex message_regex(message_regex_expr);

class subscriber
    : public cis::subscriber_interface
{
public:
    subscriber(
            std::string session_id,
            const cis1::proto_utils::transaction& tr)
        : session_id_(session_id)
        , tr_(tr)
    {}

    void log_entry(const cis1::cwu::log_entry& dto) override
    {
        dto::cis_session_log_entry res;

        res.session_id = session_id_;
        res.time = dto.time;

        if(     std::smatch smatch;
                std::regex_match(dto.message, smatch, message_regex))
        {
            res.action = smatch[1];
            res.message = smatch[2];
        }
        else
        {
            res.message = dto.message;
        }

        return tr_.send(res);
    }

    void session_not_established() override
    {
        dto::cis_session_not_established res;
        res.session_id = session_id_;

        tr_.send(res);

        unsubscribe();
    }

    void session_closed() override
    {
        dto::cis_session_closed res;
        res.session_id = session_id_;

        tr_.send(res);

        unsubscribe();
    }

    void unsubscribe() override
    {
        session_.reset();

        if(auto queue = tr_.get_queue().lock(); queue)
        {
            queue->remove_close_handler(subscribe_id_);
        }
    }

    void set_session(std::shared_ptr<cis::session_interface> session)
    {
        session_ = session;
    }

    void set_subscribe_id(uint32_t id)
    {
        subscribe_id_ = id;
    }

private:
    std::string session_id_;
    cis1::proto_utils::transaction tr_;
    std::shared_ptr<cis::session_interface> session_ = nullptr;
    uint32_t subscribe_id_;
};

void session_subscribe(
        cis::cis_manager_interface& cis_manager,
        request_context& ctx,
        const dto::cis_session_subscribe& req,
        cis1::proto_utils::transaction tr)
{
    auto sub = std::make_shared<subscriber>(
            req.session_id,
            tr);

    cis_manager.subscribe_to_session(
            req.session_id,
            ctx.session_id,
            sub);

    if(auto queue = tr.get_queue().lock(); queue)
    {
        auto id = queue->add_close_handler([sub](){});

        sub->set_subscribe_id(id);
    }
}

} // namespace handlers

} // namespace websocket
