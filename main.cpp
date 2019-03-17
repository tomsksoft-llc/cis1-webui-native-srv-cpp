#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <cstdlib>

#include "net/listener.h"
#include "net/http_session.h"
#include "net/queued_websocket_session.h"
#include "net/router.h"
#include "fail.h"
#include "response.h"
#include "file_handler.h"
#include "login_handler.h"
#include "projects_handler.h"
#include "cis_dirs.h"
#include "init.h"
#include "websocket_handler.h"
#include "rights_manager.h"
#include "file_util.h"

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

int main(int argc, char* argv[])
{
    // Check command line arguments.
    auto [address,
          port,
          cis_address,
          cis_port,
          doc_root,
          cis_root] = parse_args(argc, argv);
    cis::set_root_dir(cis_root.c_str());
    // The io_context is required for all I/O
    net::io_context ioc{};

    auto authenticate_fn = 
        [](const std::string& uname, const std::string& pass)
        {
            if(uname == "uname" && pass == "pass")
            {
                return "SomeToken";
            }
            else
            {
                return "";
            }
        };
    auto authorize_fn = 
        [](const std::string& token)
        {
            if(token == "SomeToken")
            {
                return true;
            }
            else
            {
                return false;
            }
        };
    rights_manager rm;
    auto passwd_path = path_cat(std::getenv("HOME"), "/rights.pwd");
    /*
    rm.add_resource("projects.internal.read", true);
    rm.add_resource("projects.internal.write", false);
    rm.set_right("enjection", "projects.internal.read", true);
    rm.set_right("enjection", "projects.internal.write", true);
    rm.set_right("david", "projects.internal.write", true);
    */

    rm.load_from_file(passwd_path);

    file_handler fh(doc_root);
    projects_handler ph;
    login_handler lh(authenticate_fn, authorize_fn);
    // Example of basic websocket handler
    websocket_handler ws_handler;

    ws_handler.add_event(1, [&authenticate_fn](
                const boost::property_tree::ptree& data,
                websocket_queue& queue)
            {
                //TODO validate
                auto login = data.get<std::string>("login");
                auto pass = data.get<std::string>("pass");
                std::string token = authenticate_fn(login, pass);
                if(!token.empty())
                {
                    auto reply = std::make_shared<std::string>(token);
                    queue.send_text(
                            boost::asio::buffer(reply->data(), reply->size()), [reply](){});
                }
                else
                {
                    auto reply = std::make_shared<std::string>("wrong user or password");
                    queue.send_text(
                            boost::asio::buffer(reply->data(), reply->size()), [reply](){});
                }
            });

    auto ws_msg_handler = 
        [&ws_handler](
                bool text,
                beast::flat_buffer& buffer,
                size_t bytes_transferred,
                websocket_queue& queue)
        {
            ws_handler.handle(text, buffer, bytes_transferred, queue);
        };

    // Create router for requests
    auto base_router = std::make_shared<router>();
    base_router->add_route("/", 
            [&fh, &lh](
                http::request<http::string_body>&& req,
                http_session::queue& queue)
            {
                if(!lh.authorize(req))
                {
                    fh.handle(std::move(req), queue, "/index.html");
                }
                else
                {
                    fh.handle(std::move(req), queue, "/index_auth_ok.html");
                }
            });
    base_router->add_route("/projects", 
            [&ph](auto&&... args)
            {
                ph.get_projects(std::forward<decltype(args)>(args)...);
            });
    /*
    base_router->add_route("/run/<string>/<string>"_R, 
            //-> "^/run/([^/?#]+)/([^/?#]+)" 
            //(substitute "<string>" to "([^/?#]+)", make {std::string, std::string} arg
            // get groups 0 to ..n and assign it to arg)
            [&ph, &ioc](
                http::request<http::string_body>&& req,
                http_session::queue& queue,
                const std::string& project
                const std::string& job)
            {
                ph.run(ioc, "internal", "core_test");
            });*/
    base_router->add_route("/login", 
            [&lh](auto&&... args)
            {
                lh.handle(std::forward<decltype(args)>(args)...);
            });
    base_router->add_catch_route(
            [&fh](auto&&... args)
            {
                fh.handle(std::forward<decltype(args)>(args)...);
            });
    base_router->add_ws_route("/ws", 
            [&ws_msg_handler](auto&&... args)
            {
                queued_websocket_session::accept_handler(std::forward<decltype(args)>(args)..., ws_msg_handler);
            });

    // Create and launch a listening port
    auto accept_handler = 
    [&base_router](tcp::socket&& socket){
        std::make_shared<http_session>(
            std::move(socket),
            base_router)->run();
    };
    auto l = std::make_shared<listener>(
        ioc,
        accept_handler);
    beast::error_code ec;
    l->listen(tcp::endpoint{address, port}, ec);
    if(ec)
    {
        return EXIT_FAILURE;
    }
    l->run();
    l.reset();

    auto cis_router = std::make_shared<router>();
    cis_router->add_route("/projects",
            [&ph](http::request<http::string_body>&& req,
                http_session::queue& queue)
            {
                ph.update();
                return queue.send(response::accepted(std::move(req)));
            });
    auto cis_accept_handler = 
    [&cis_router](tcp::socket&& socket){
        std::make_shared<http_session>(
            std::move(socket),
            cis_router)->run();
    };
    l = std::make_shared<listener>(
        ioc,
        cis_accept_handler);
    l->listen(tcp::endpoint{cis_address, cis_port}, ec);
    if(ec)
    {
        return EXIT_FAILURE;
    }
    l->run();
    l.reset();

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
