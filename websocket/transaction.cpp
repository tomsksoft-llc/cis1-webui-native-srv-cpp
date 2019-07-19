#include "transaction.h"

#include <rapidjson/writer.h>

namespace websocket
{

transaction::transaction(
        const std::shared_ptr<net::websocket_queue>& queue,
        uint64_t transaction_id,
        int32_t default_error_id)
    : transaction_id_(transaction_id)
    , queue_(queue)
    , default_error_id_(default_error_id)
{}

void transaction::send_error(const std::string& err)
{
    if(auto queue = queue_.lock(); queue)
    {
        rapidjson::Document d;

        prepare_response(d, default_error_id_);

        d["data"].AddMember(
                rapidjson::Value().SetString("errorMessage"),
                rapidjson::Value().SetString(
                        err.c_str(),
                        err.length(),
                        d.GetAllocator()),
                d.GetAllocator());

        auto buffer = std::make_shared<rapidjson::StringBuffer>();
        rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
        d.Accept(writer);

        queue->send_text(
                boost::asio::const_buffer(
                        buffer->GetString(),
                        buffer->GetSize()),
                [buffer](){});
    }
}

void transaction::prepare_response(rapidjson::Document& doc, int32_t id)
{
    doc.SetObject();
    doc.AddMember(
            rapidjson::Value().SetString("eventId"),
            rapidjson::Value().SetInt(id),
            doc.GetAllocator());
    doc.AddMember(
            rapidjson::Value().SetString("transactionId"),
            rapidjson::Value().SetUint(transaction_id_),
            doc.GetAllocator());
    doc.AddMember(
            rapidjson::Value().SetString("data"),
            rapidjson::Value().SetObject(),
            doc.GetAllocator());
}

void transaction::send(
        const std::shared_ptr<net::websocket_queue> queue,
        const rapidjson::Value& v)
{
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    v.Accept(writer);

    queue->send_text(
            boost::asio::const_buffer(
                    buffer->GetString(),
                    buffer->GetSize()),
            [buffer](){});
}

} // namespace websocket
