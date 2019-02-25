#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <cstdlib>

#include "listener.h"
#include "http_session.h"
#include "fail.h"
#include "queued_websocket_session.h"
#include "file_handler.h"

namespace pt = boost::property_tree;
namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

int main(int argc, char* argv[])
{
    net::ip::address address;
    unsigned short port;
    std::string doc_root;
    std::string cis_root;
    // Check command line arguments.
    if(argc == 2)
    {
        pt::ptree pt;
        pt::ini_parser::read_ini(argv[1], pt);
        address = net::ip::make_address(pt.get<std::string>("http.ip"));
        port = pt.get<unsigned short>("http.port");
        doc_root = pt.get<std::string>("http.doc_root");
        auto opt_cis_root = pt.get_optional<std::string>("cis.base_dir");
        if(opt_cis_root)
        {
            cis_root = opt_cis_root.value();
        }
        else
        {
            cis_root = std::getenv("cis_base_dir");
        }
    }
    else
    {
        address = net::ip::make_address("127.0.0.1");
        port = static_cast<unsigned short>(8080);
        doc_root = ".";
        cis_root = std::getenv("cis_base_dir");
    }

    // The io_context is required for all I/O
    net::io_context ioc{};

    file_handler fh(doc_root);
    // Example of basic websocket handler
    auto ws_msg_handler = 
        [](bool text, beast::flat_buffer& buffer, size_t bytes_transferred, websocket_queue& queue)
        {
            char* tempchar = new char[bytes_transferred + 1];
            boost::asio::buffer_copy(boost::asio::buffer(tempchar, bytes_transferred), buffer.data(), bytes_transferred);
            tempchar[bytes_transferred] = '\0';
            if(text)
            {
                std::cout << tempchar << std::endl;
            }
            else
            {
                std::cout << "got binary message!" << std::endl;
            }
            delete[] tempchar;
            auto reply = std::make_shared<std::string>("reply text");
            // Using lambda for control reply lifetime
            queue.send_text(boost::asio::buffer(reply->data(), reply->size()), [reply](){});
            queue.send_text(boost::asio::buffer(reply->data(), reply->size()), [reply](){});
        };

    // Create router for requests
    auto base_router = std::make_shared<router>();
    base_router->add_route("/", 
            [&fh](auto&&... args)
            {
                fh.handle(std::forward<decltype(args)>(args)..., "/index.html");
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
