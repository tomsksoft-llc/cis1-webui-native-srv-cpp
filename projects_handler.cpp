#include "projects_handler.h"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "net/http_session.h"
#include "file_util.h"
#include "dirs.h"
#include "response.h"

projects_handler::projects_handler()
    : projects_(path_cat(cis::get_root_dir(), cis::PROJECTS))
{
    projects_.fetch();
}

void projects_handler::operator()(
            const rapidjson::Document& data,
            websocket_queue& queue,
            web_app::context_t& /*ctx*/)
{
    rapidjson::Document document;
    rapidjson::Value value;
    document.SetObject();
    value.SetInt(22);
    document.AddMember("eventId", value, document.GetAllocator());
    rapidjson::Value data_value;
    data_value.SetObject();
    auto projects_json = projects_.to_json();
    value.CopyFrom(projects_json, document.GetAllocator());
    data_value.AddMember("projects", value, document.GetAllocator());
    value.SetString("");
    data_value.AddMember("errorMessage", value, document.GetAllocator());
    document.AddMember("data", data_value, document.GetAllocator());
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    document.Accept(writer);
    queue.send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}

void projects_handler::run(
        boost::asio::io_context& ctx,
        const std::string& project,
        const std::string& job)
{
    ::run_job(ctx, project, job);
}  

web_app::handle_result projects_handler::update(
        web_app::request_t& req,
        web_app::queue_t& queue,
        web_app::context_t& /*ctx*/)
{
    beast::error_code ec;
        // Handle an unknown error
    if(ec)
    {
        queue.send(response::server_error(std::move(req), ec.message()));
        return web_app::handle_result::done;
    
    }

    projects_.fetch();

    http::response<http::empty_body> res{
        http::status::ok,
        req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.prepare_payload();
    res.keep_alive(req.keep_alive());
    queue.send(std::move(res));
    return web_app::handle_result::done;
}
