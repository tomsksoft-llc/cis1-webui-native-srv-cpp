#include "websocket_event_handlers.h"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace std::string_literals;

void handle_auth(
        std::shared_ptr<auth_manager> authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        web_app::context_t& ctx)
{
    auto login = data.HasMember("login") && data["login"].IsString() ?
        data["login"].GetString() : ""s;
    auto pass = data.HasMember("pass") && data["pass"].IsString() ?
        data["pass"].GetString() : ""s;
    std::string token = authentication_handler->authenticate(login, pass);

    rapidjson::Document document;
    rapidjson::Value value;
    document.SetObject();
    value.SetInt(2);
    document.AddMember("eventId", value, document.GetAllocator());
    rapidjson::Value data_value;
    data_value.SetObject();
    value.SetString(token.c_str(), token.length(), document.GetAllocator());
    data_value.AddMember("token", value, document.GetAllocator());
    if(!token.empty())
    {
        ctx["user"] = std::string(login);
        value.SetString("");
    }
    else
    {
        value.SetString("Wrong username or password");
    }
    data_value.AddMember("errorMessage", value, document.GetAllocator());
    document.AddMember("data", data_value, document.GetAllocator());
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    document.Accept(writer);
    queue.send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}

void handle_token(
        std::shared_ptr<auth_manager> authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        web_app::context_t& ctx)
{
    auto token = data.HasMember("token") && data["token"].IsString() ?
        data["token"].GetString() : ""s;
    std::string username = authentication_handler->authenticate(token);

    rapidjson::Document document;
    rapidjson::Value value;
    document.SetObject();
    value.SetInt(4);
    document.AddMember("eventId", value, document.GetAllocator());
    rapidjson::Value data_value;
    data_value.SetObject();
    if(!username.empty())
    {
        ctx["user"] = username;
        value.SetString("");
    }
    else
    {
        value.SetString("Invalid token");
    }
    data_value.AddMember("errorMessage", value, document.GetAllocator());
    document.AddMember("data", data_value, document.GetAllocator());
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    document.Accept(writer);
    queue.send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}
