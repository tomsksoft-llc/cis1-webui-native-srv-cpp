#pragma once

#include <functional>
#include <map>
#include <memory>

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/buffers_to_string.hpp>

#include <rapidjson/document.h>

#include "queue_interface.h"
#include "request_context.h"
#include "transaction.h"

#include "tpl_reflect/json_engine.h"

namespace websocket
{

class event_dispatcher
{
public:
    template <class ReqType>
    using default_event_handler_t = void(
            request_context&,
            const ReqType&,
            transaction);

    template <class ReqType>
    struct FnWrapper
    {
        template <class Fn>
        FnWrapper(Fn&& fn)
            : fn_(fn)
        {}

        std::function<default_event_handler_t<ReqType>> fn_;
    };

    void dispatch(
            request_context& ctx,
            bool text,
            boost::beast::flat_buffer& buffer,
            size_t bytes_transferred,
            const std::shared_ptr<queue_interface>& queue);

    template <class ReqType>
    void add_event_handler(
            const std::function<default_event_handler_t<ReqType>>& cb)
    {
        const auto& conv = ReqType::get_converter();
        
        event_handlers_.insert({
                conv.template name<json::engine>(),
                [cb](
                     request_context& ctx,
                     const rapidjson::Value& json,
                     transaction tr)
                {
                    ReqType req;
                    const auto& conv = ReqType::get_converter();

                    if(    conv.template has<json::engine>(json)
                        && conv.template get<json::engine>(json, req))
                    {
                        cb(
                                ctx,
                                req,
                                std::move(tr));
                    }
                    else
                    {
                        tr.send_error("Invalid json.");
                    }
                }});
    }
private:
    using event_handler_t = void(
            request_context& ctx,
            const rapidjson::Value& json,
            transaction tr);

    std::map<std::string, std::function<event_handler_t>> event_handlers_;
};

} // namespace websocket
