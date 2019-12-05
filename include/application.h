/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <cis1_cwu_transport/ccwu_tcp_server.h>

#include "init.h"
#include "auth_manager.h"
#include "rights_manager.h"
#include "database.h"
#include "cis/cis_manager.h"
#include "http/handlers_chain.h"
#include "http/router.h"
#include "http/file_handler.h"
#include "http/multipart_form_handler.h"
#include "http/download_handler.h"
#include "http/webhooks_handler.h"
#include "configuration_manager.h"

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace asio = boost::asio;                   // from <boost/asio.hpp>

class application
{
protected:
    struct private_constructor_delegate_t;

public:
    static std::optional<application> create(
            boost::asio::io_context& ioc,
            std::unique_ptr<configuration_manager> config,
            std::error_code& ec);

    template <class... Args>
    application(
            const private_constructor_delegate_t&,
            Args&&... args)
        : application(std::forward<Args>(args)...)
    {}

    void run();
    void stop();

protected:
    struct private_constructor_delegate_t
    {
        explicit private_constructor_delegate_t();
    };

private:
    asio::io_context& ioc_;
    std::shared_ptr<http::handlers_chain> app_;
    std::shared_ptr<cis1::cwu::tcp_server> cis_app_;
    std::unique_ptr<configuration_manager> config_;
    std::unique_ptr<database::database_wrapper> db_;
    std::unique_ptr<cis::cis_manager> cis_;
    std::unique_ptr<auth_manager> auth_manager_;
    std::unique_ptr<rights_manager> rights_manager_;
    std::unique_ptr<http::file_handler> files_;
    std::unique_ptr<http::multipart_form_handler> upload_handler_;
    std::unique_ptr<http::download_handler> download_handler_;
    std::unique_ptr<http::webhooks_handler> webhooks_handler_;

    application(
            boost::asio::io_context& ioc,
            const std::shared_ptr<http::handlers_chain>& app,
            const std::shared_ptr<cis1::cwu::tcp_server>& cis_app,
            std::unique_ptr<configuration_manager> config,
            std::unique_ptr<database::database_wrapper> db,
            std::unique_ptr<cis::cis_manager> cis,
            std::unique_ptr<auth_manager> auth_manager_arg,
            std::unique_ptr<rights_manager> rights_manager_arg,
            std::unique_ptr<http::file_handler> files,
            std::unique_ptr<http::multipart_form_handler> upload_handler,
            std::unique_ptr<http::download_handler> download_handler,
            std::unique_ptr<http::webhooks_handler> webhooks_handler);
};
