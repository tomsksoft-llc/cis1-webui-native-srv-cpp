#include <functional>
#ifdef ENABLE_THREADING
#include <vector>
#include <thread>
#endif

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "init.h"
#include "db_dirs.h"
#include "cis/dirs.h"
#include "request_context.h"
// Business logic
#include "auth_manager.h"
#include "rights_manager.h"
#include "cis/project_list.h"
// Middleware
#include "router.h"
#include "cookie_parser.h"
#include "file_handler.h"
// HTTP handlers
#include "http_handlers_chain.h"
#include "http_handlers.h"
// WebSocket handlers
#include "websocket_event_dispatcher.h"
#include "websocket_event_handlers.h"

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace asio = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
using namespace std::placeholders;
namespace wsh = websocket::handlers;

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
    asio::io_context ioc{};
   
    // Configure and run public http interface
    auto app = std::make_shared<http_handlers_chain>();
    
    auto authentication_handler = std::make_shared<auth_manager>();
    auto authorization_handler = std::make_shared<rights_manager>();
    auto files = std::make_shared<file_handler>(doc_root);
    auto projects = std::make_shared<cis::project_list>(ioc);
    projects->run();
    auto public_router = std::make_shared<http_router>();
    auto& index_route = public_router->add_route("/");
    index_route.append_handler(
            std::bind(&file_handler::single_file, files, _1, _2, _3, _4, "/index.html"));
    public_router->add_catch_route()
        .append_handler(
                std::bind(&file_handler::operator(), files, _1, _2, _3, _4));
    auto ws_router = std::make_shared<websocket_router>();
    auto& ws_route = ws_router->add_route("/ws(\\?.+)*");

    websocket_event_dispatcher ws_dispatcher;
    ws_dispatcher.add_event_handler(ws_request_id::auth_login_pass,
            std::bind(&wsh::authenticate, authentication_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::auth_token,
            std::bind(&wsh::token, authentication_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::logout,
            std::bind(&wsh::logout, authentication_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::change_pass,
            std::bind(&wsh::change_pass, authentication_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::list_users,
            std::bind(&wsh::list_users, authentication_handler, authorization_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::get_user_permissions,
            std::bind(&wsh::get_user_permissions, authorization_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::set_user_permissions,
            std::bind(&wsh::set_user_permissions, authorization_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::change_group,
            std::bind(&wsh::change_group, authentication_handler, authorization_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::disable_user,
            std::bind(&wsh::disable_user, authentication_handler, authorization_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::generate_api_key,
            std::bind(&wsh::generate_api_key, authentication_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::list_projects,
            std::bind(&wsh::list_projects, projects, authorization_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::get_project_info,
            std::bind(&wsh::get_project_info, projects, authorization_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::get_job_info,
            std::bind(&wsh::get_job_info, projects, authorization_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::run_job,
            std::bind(&wsh::run_job, projects, authorization_handler, std::ref(ioc), _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::rename_job,
            std::bind(&wsh::rename_job, projects, authorization_handler, _1, _2, _3, _4));
    ws_dispatcher.add_event_handler(ws_request_id::get_build_info,
            std::bind(&wsh::get_build_info, projects, authorization_handler, _1, _2, _3, _4));

    ws_route.append_handler([&ws_dispatcher](
                http::request<http::empty_body>& req,
                tcp::socket& socket,
                request_context& ctx)
            {
            net::queued_websocket_session::accept_handler(
                        std::move(socket),
                        std::move(req),
                        std::bind(
                            &websocket_event_dispatcher::dispatch,
                            ws_dispatcher,
                            ctx,
                            _1, _2, _3, _4));
                return handle_result::done;
            });

    app->append_handler(&cookie_parser::parse);
    app->append_handler(
            std::bind(
                &handle_authenticate,
                authentication_handler,
                _1, _2, _3, _4));
    app->append_handler(
            std::bind(
                &http_router::operator(),
                public_router,
                _1, _2, _3, _4));
    app->append_ws_handler(
            std::bind(
                &websocket_router::operator(),
                ws_router,
                _1, _2, _3));
    app->listen(ioc, tcp::endpoint{address, port});

    // Configure and run cis http interface
    auto cis_app = std::make_shared<http_handlers_chain>();
    auto cis_router = std::make_shared<http_router>();
    cis_app->append_handler(
            std::bind(
                &http_router::operator(),
                cis_router,
                _1, _2,_3, _4));
    auto& projects_route = cis_router->add_route("/projects");
    projects_route.append_handler(
            std::bind(
                &handle_update_projects,
                projects,
                _1, _2, _3, _4));
    cis_app->listen(ioc, tcp::endpoint{cis_address, cis_port});

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    asio::signal_set signals(ioc, SIGINT, SIGTERM);
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
