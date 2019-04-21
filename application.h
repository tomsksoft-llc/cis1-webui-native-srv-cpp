#pragma once

#include "init.h"
#include "auth_manager.h"
#include "rights_manager.h"
#include "database.h"
#include "http/handlers_chain.h"
#include "http/router.h"
#include "http/file_handler.h"
#include "cis/project_list.h"

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace asio = boost::asio;                   // from <boost/asio.hpp>

class application
{
    init_params params_;
    database::database db_;
    asio::io_context ioc_;
    asio::signal_set signals_;
    std::shared_ptr<http::handlers_chain> app_;
    std::shared_ptr<http::handlers_chain> cis_app_;
    std::shared_ptr<http::file_handler> files_;
    std::shared_ptr<cis::project_list> projects_;
    std::shared_ptr<auth_manager> auth_manager_;
    std::shared_ptr<rights_manager> rights_manager_;

    void init_app();
    void init_cis_app();
    std::shared_ptr<http_router> make_public_http_router();
    std::shared_ptr<websocket_router> make_ws_router();
    std::shared_ptr<http_router> make_cis_http_router();
public:
    application(const init_params& params);
    void run();
    void stop();
};
