#include <functional>
#ifdef ENABLE_THREADING
#include <vector>
#include <thread>
#endif

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "init.h"
#include "dirs.h"
#include "request_context.h"
#include "web_app.h"
// Business logic
#include "auth_manager.h"
// Middleware
#include "router.h"
#include "cookie_parser.h"
#include "file_handler.h"
// HTTP handlers
#include "projects_handler.h"
// WebSocket handlers
#include "websocket_handler.h"
#include "websocket_event_handlers.h"

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
using namespace std::placeholders;

int main(int argc, char* argv[])
{
    // Load config
    auto [address,
          port,
          cis_address,
          cis_port,
          doc_root,
          cis_root,
          db_path] = parse_args(argc, argv);
    cis::set_root_dir(cis_root.c_str());
    db::set_root_dir(db_path.c_str());
    // The io_context is required for all I/O
    net::io_context ioc{};
   
    // Configure and run public http interface
    auto app = std::make_shared<web_app>(ioc);
    
    auto authentication_handler = std::make_shared<auth_manager>();
    auto files = std::make_shared<file_handler>(doc_root);
    auto projects = std::make_shared<projects_handler>();
    auto public_router = std::make_shared<http_router>();
    auto& index_route = public_router->add_route("/");
    index_route.append_handler(
            std::bind(&file_handler::single_file, files, _1, _2, _3, "/index.html"));
    public_router->add_catch_route()
        .append_handler(
                std::bind(&file_handler::operator(), files, _1, _2, _3));
    auto ws_router = std::make_shared<websocket_router>();
    auto& ws_route = ws_router->add_route("/ws(\\?.+)*");
    websocket_handler ws_handler;
    ws_handler.add_event_handler(ws_request_id::auth_login_pass, std::bind(&handle_auth, authentication_handler, _1, _2, _3));
    ws_handler.add_event_handler(ws_request_id::auth_token, std::bind(&handle_token, authentication_handler, _1, _2, _3));
    ws_handler.add_event_handler(ws_request_id::auth_logout, std::bind(&handle_logout, authentication_handler, _1, _2, _3));
    ws_handler.add_event_handler(ws_request_id::projects_list, std::bind(&projects_handler::get_project_list, projects, _1, _2, _3));
    ws_handler.add_event(23, std::bind(&projects_handler::get_subproject_list, projects, _1, _2, _3));

    ws_route.append_handler([&ws_handler](
                web_app::request_t& req,
                tcp::socket& socket,
                request_context& ctx)
            {
                queued_websocket_session::accept_handler(
                        std::move(socket),
                        std::move(req),
                        std::bind(&websocket_handler::handle, ws_handler, ctx, _1, _2, _3, _4));
                return handle_result::done;
            });

    app->append_handler(&cookie_parser::parse);
    app->append_handler(
            std::bind(
                &auth_manager::operator(),
                authentication_handler,
                _1, _2, _3));
    app->append_handler(
            std::bind(
                &http_router::operator(),
                public_router,
                _1, _2, _3));
    app->append_ws_handler(
            std::bind(
                &websocket_router::operator(),
                ws_router,
                _1, _2, _3));
    app->listen(tcp::endpoint{address, port});

    // Configure and run cis http interface
    auto cis_app = std::make_shared<web_app>(ioc);
    auto cis_router = std::make_shared<http_router>();
    cis_app->append_handler(
            std::bind(
                &http_router::operator(),
                cis_router,
                _1, _2,_3));
    auto& projects_route = cis_router->add_route("/projects");
    projects_route.append_handler(
            std::bind(
                &projects_handler::update,
                projects,
                _1, _2, _3));
    cis_app->listen(tcp::endpoint{cis_address, cis_port});

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&](beast::error_code const&, int)
        {
            ioc.stop();
        });
#ifdef ENABLE_THREADING
    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
    {
        v.emplace_back(
        [&ioc]
        {
            ioc.run();
        });
    }
#endif   

    ioc.run();

#ifdef ENABLE_THREADING
    // Block until all the threads exit
    for(auto& t : v)
    {
        t.join();
    }
#endif   
    return EXIT_SUCCESS;
}
