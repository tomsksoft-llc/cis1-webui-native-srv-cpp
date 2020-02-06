/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "application.h"

#include <functional>
#include <iomanip>

#include <cis1_proto_utils/event_dispatcher.h>
#include <cis1_cwu_protocol/protocol.h>

#include "net/queued_websocket_session.h"
#include "http/error_handler.h"
#include "http/cookie_parser.h"
#include "http/common_handlers.h"
#include "http/url.h"
#include "websocket/event_handlers.h"
#include "cis/dirs.h"
#include "openssl_wrapper/openssl_wrapper.h"
#include "error_code.h"

using namespace std::placeholders;              // from <functional>

std::shared_ptr<http_router> make_http_router(
        http::file_handler& files,
        http::multipart_form_handler& upload_handler,
        http::download_handler& download_handler,
        http::webhooks_handler& webhooks_handler)
{
    auto router = std::make_shared<http_router>();

    router->add_route(
            url::make() / CT_STRING("upload") / url::bound_string() / url::string(),
            [&upload_handler](auto&& ...args)
            {
                return upload_handler.upload(std::forward<decltype(args)>(args)...);
            });

    router->add_route(
            url::make() / CT_STRING("upload") / url::bound_string(),
            [&upload_handler](auto&& ...args)
            {
                return upload_handler.upload(std::forward<decltype(args)>(args)...);
            });

    router->add_route(
            url::make() / CT_STRING("replace") / url::bound_string() / url::string(),
            [&upload_handler](auto&& ...args)
            {
                return upload_handler.replace(std::forward<decltype(args)>(args)...);
            });

    router->add_route(
            url::make() / CT_STRING("replace") / url::bound_string(),
            [&upload_handler](auto&& ...args)
            {
                return upload_handler.replace(std::forward<decltype(args)>(args)...);
            });

    router->add_route(
            url::make() / CT_STRING("download") / url::bound_string() / url::string(),
            [&download_handler](auto&& ...args)
            {
                return download_handler(std::forward<decltype(args)>(args)...);
            });

    auto webhooks_url = url::make() /
            CT_STRING("users") / url::bound_string() /
            CT_STRING("webhooks");

    router->add_route(
            webhooks_url / CT_STRING("github") /
                    url::bound_string() / url::bound_string() << url::query_string(),
            [&whh = webhooks_handler](auto&& ...args)
            {
                return whh(
                        std::forward<decltype(args)>(args)...,
                        http::webhooks_handler::api::github);
            });

    router->add_route(
            webhooks_url / CT_STRING("gitlab") /
                    url::bound_string() / url::bound_string() << url::query_string(),
            [&whh = webhooks_handler](auto&& ...args)
            {
                return whh(
                        std::forward<decltype(args)>(args)...,
                        http::webhooks_handler::api::gitlab);
            });

    router->add_route(
            webhooks_url / CT_STRING("plain") /
                    url::bound_string() / url::bound_string() << url::query_string(),
            [&whh = webhooks_handler](auto&& ...args)
            {
                return whh(
                        std::forward<decltype(args)>(args)...,
                        http::webhooks_handler::api::plain);
            });

    std::function<http::handle_result(
            beast::http::request<beast::http::empty_body>&,
            request_context&,
            net::http_session::request_reader&,
            net::http_session::queue&)> cb = std::bind(
            &http::file_handler::single_file,
            std::ref(files),
            _1, _2, _3, _4,
            "/index.html");

    router->add_route(url::root(), cb);

    router->add_route(url::make() / url::ignore(),
            [&files](auto&& ...args)
            {
                return files(std::forward<decltype(args)>(args)...);
            });

    return router;
}

std::shared_ptr<websocket_router> make_ws_router(
        configuration_manager& config_,
        cis::cis_manager& cis_,
        auth_manager& auth_manager_,
        rights_manager& rights_manager_)
{
    namespace ws = websocket;
    namespace wsh = ws::handlers;
    auto router = std::make_shared<websocket_router>();

    cis1::proto_utils::event_dispatcher<request_context&> dispatcher;
    dispatcher.add_event_handler<ws::dto::auth_login_pass>(
            std::bind(&wsh::authenticate,
                    std::ref(auth_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::auth_token>(
            std::bind(&wsh::token,
                    std::ref(auth_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::auth_logout>(
            std::bind(&wsh::logout,
                    std::ref(auth_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_auth_change_pass>(
            std::bind(&wsh::change_pass,
                    std::ref(auth_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_auth_add>(
            std::bind(&wsh::add_user,
                      std::ref(auth_manager_),
                      std::ref(rights_manager_),
                      _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_list>(
            std::bind(&wsh::list_users,
                std::ref(auth_manager_),
                std::ref(rights_manager_),
                _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_permissions_projects_get>(
            std::bind(&wsh::get_user_permissions_projects,
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_permissions_projects_set>(
            std::bind(&wsh::set_user_permissions_projects,
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_auth_change_group>(
            std::bind(&wsh::change_group,
                    std::ref(auth_manager_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_auth_ban>(
            std::bind(&wsh::ban_user,
                    std::ref(auth_manager_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_auth_unban>(
            std::bind(&wsh::unban_user,
                    std::ref(auth_manager_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_api_key_generate>(
            std::bind(&wsh::generate_api_key,
                    std::ref(auth_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_api_key_get>(
            std::bind(&wsh::get_api_key,
                    std::ref(auth_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_api_key_remove>(
            std::bind(&wsh::remove_api_key,
                    std::ref(auth_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_project_list_get>(
            std::bind(&wsh::list_projects,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_project_info>(
            std::bind(&wsh::get_project_info,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_project_add>(
            std::bind(&wsh::add_cis_project,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_project_remove>(
            std::bind(&wsh::remove_cis_project,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_job_add>(
            std::bind(&wsh::add_cis_job,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_job_remove>(
            std::bind(&wsh::remove_cis_job,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_build_remove>(
            std::bind(&wsh::remove_cis_build,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_job_info>(
            std::bind(&wsh::get_job_info,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_job_run>(
            std::bind(&wsh::run_job,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_build_info>(
            std::bind(&wsh::get_build_info,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::fs_entry_info>(
            std::bind(&wsh::get_fs_entry_info,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::fs_entry_refresh>(
            std::bind(&wsh::refresh_fs_entry,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::fs_entry_remove>(
            std::bind(&wsh::remove_fs_entry,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::fs_entry_move>(
            std::bind(&wsh::move_fs_entry,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::fs_entry_new_dir>(
            std::bind(&wsh::new_directory,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::fs_entry_list>(
            std::bind(&wsh::list_directory,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::fs_entry_set_executable_flag>(
            std::bind(&wsh::set_fs_entry_executable_flag,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::group_default_permissions_get>(
            std::bind(&wsh::get_group_default_permissions,
                      std::ref(auth_manager_),
                      std::ref(rights_manager_),
                      _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::group_default_permissions_set>(
            std::bind(&wsh::set_group_default_permissions,
                      std::ref(auth_manager_),
                      std::ref(rights_manager_),
                      _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_cron_add>(
            [&cis_, &rights_manager_](auto&& ...args){
                    wsh::add_cis_cron(
                            cis_,
                            rights_manager_,
                            std::forward<decltype(args)>(args)...);
            });
    dispatcher.add_event_handler<ws::dto::cis_cron_remove>(
            std::bind(&wsh::remove_cis_cron,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_cron_list>(
            std::bind(&wsh::list_cis_cron,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::cis_session_subscribe>(
            [&cis_](auto&& ...args){
                    wsh::session_subscribe(
                            cis_,
                            std::forward<decltype(args)>(args)...);
            });
    dispatcher.add_event_handler<ws::dto::cis_session_unsubscribe>(
            [&cis_](auto&& ...args){
                    wsh::session_unsubscribe(
                            cis_,
                            std::forward<decltype(args)>(args)...);
            });
    dispatcher.add_event_handler<ws::dto::system_version_info>(
            [&config_](auto&& ...args){
                    wsh::get_system_version(
                            config_,
                            std::forward<decltype(args)>(args)...);
            });

    std::function <http::handle_result(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        tcp::socket& socket)> cb = [dispatcher](
                beast::http::request<beast::http::empty_body>& req,
                request_context& ctx,
                tcp::socket& socket) mutable
                {
                    static uint64_t session_id = 0;

                    ctx.session_id = session_id;

                    ++session_id;

                    net::queued_websocket_session::accept_handler(
                            std::move(socket),
                            std::move(req),
                            std::bind(
                                &cis1::proto_utils::event_dispatcher<request_context&>::dispatch,
                                std::ref(dispatcher),
                                ctx,
                                _1, _2, _3, _4));

                    return http::handle_result::done;
                };

    router->add_route(url::make() / CT_STRING("ws"), cb);

    return router;
}

void init_public_app(
        const std::shared_ptr<http::handlers_chain>& public_app,
        const std::shared_ptr<http::error_handler>& error_handler,
        auth_manager& auth_manager_,
        const std::shared_ptr<http_router>& http_router_arg,
        const std::shared_ptr<websocket_router>& ws_router)
{
    public_app->set_error_handler(
            std::bind(
                    &http::error_handler::operator(),
                    error_handler,
                    _1, _2, _3));

    public_app->append_handler(&http::cookie_parser::parse);

    public_app->append_handler(
            std::bind(
                    &http::handle_authenticate,
                    std::ref(auth_manager_),
                    _1, _2, _3, _4));

    public_app->append_handler(
            std::bind(
                    &http_router::operator(),
                    http_router_arg,
                    _1, _2, _3, _4));

    public_app->append_ws_handler(
            std::bind(
                    &websocket_router::operator(),
                    ws_router,
                    _1, _2, _3));
}

void init_cis_app(
        const std::shared_ptr<cis1::cwu::tcp_server>& cis_app,
        cis::cis_manager& cis)
{
    struct session_context
    {
        std::shared_ptr<cis::session_interface> session = nullptr;
    };

    cis1::proto_utils::event_dispatcher<session_context&> dispatcher;

    dispatcher.add_event_handler<cis1::cwu::session_auth>(
            [&cis = cis](
                    session_context& ctx,
                    const cis1::cwu::session_auth& dto,
                    cis1::proto_utils::transaction tr)
            {
                ctx.session = cis.connect_to_session(dto.session_id);

                if(!ctx.session)
                {
                    if(auto queue = tr.get_queue().lock(); queue)
                    {
                        queue->close();
                    }
                }
            });

    dispatcher.add_event_handler<cis1::cwu::log_entry>(
            [](     session_context& ctx,
                    const cis1::cwu::log_entry& dto,
                    cis1::proto_utils::transaction tr)
            {
                if(!ctx.session)
                {
                    if(auto queue = tr.get_queue().lock(); queue)
                    {
                        queue->close();
                    }
                }

                ctx.session->log(dto);
            });

    cis_app->set_session_acceptor(
            [dispatcher = std::move(dispatcher)]() mutable
            {
                return [&dispatcher, ctx = session_context{}](
                            boost::asio::const_buffer buffer,
                            std::shared_ptr<cis1::proto_utils::queue_interface>
                                            queue) mutable
                        {
                            dispatcher.dispatch(ctx, true, buffer, buffer.size(), queue);
                        };
            });
}

std::optional<boost::asio::ip::tcp::endpoint> resolve_endpoint(
        boost::asio::io_context& ioc,
        const std::string& address,
        uint16_t port,
        std::error_code& ec)
{
    boost::asio::ip::tcp::resolver resolver(ioc);

    boost::system::error_code boost_ec;

    auto endpoints = resolver.resolve(
            address,
            std::to_string(port),
            boost_ec);

    if(boost_ec)
    {
        ec = cis::error_code::cant_resolve_address;

        return std::nullopt;
    }

    return *(endpoints.begin());
}

std::optional<application> application::create(
        boost::asio::io_context& ioc,
        std::unique_ptr<configuration_manager> config,
        std::error_code& ec)
{
    openssl::init();

    //apps

    auto public_app = std::make_shared<http::handlers_chain>(ioc);

    auto cis_app = std::make_shared<cis1::cwu::tcp_server>(ioc);

    auto db = database::database_wrapper::create(
            *(config->get_entry<std::filesystem::path>("db_root")) / "db.sqlite",
            config->get_entry<user_credentials>("admin_credentials"),
            config->get_entry<user_credentials>("guest_credentials"),
            ec);

    if(ec)
    {
        return std::nullopt;
    }

    config->remove_entry("admin_credentials");

    auto cis = std::make_unique<cis::cis_manager>(
            ioc,
            *config,
            *db);

    auto auth_manager_ = std::make_unique<auth_manager>(
            ioc,
            *db);

    auto rights_manager_ = std::make_unique<rights_manager>(
            ioc,
            *db);

    auto files = std::make_unique<http::file_handler>(
            (config->get_entry<std::filesystem::path>("doc_root", ec))
                    ->generic_string().c_str());

    auto upload_handler = std::make_unique<http::multipart_form_handler>(
            *(config->get_entry<std::filesystem::path>("cis_root", ec)) / cis::projects,
            *rights_manager_);

    auto download_handler = std::make_unique<http::download_handler>(
            *(config->get_entry<std::filesystem::path>("cis_root", ec)) / cis::projects,
            *rights_manager_);

    auto webhooks_handler = std::make_unique<http::webhooks_handler>(
            *auth_manager_,
            *rights_manager_,
            *cis);

    //public_app

    init_public_app(
            public_app,
            std::make_shared<http::error_handler>(),
            *auth_manager_,
            make_http_router(
                    *files,
                    *upload_handler,
                    *download_handler,
                    *webhooks_handler),
            make_ws_router(
                    *config,
                    *cis,
                    *auth_manager_,
                    *rights_manager_));

    auto endpoint = resolve_endpoint(
            ioc,
            *(config->get_entry<std::string>("public_address", ec)),
            *(config->get_entry<uint16_t>("public_port", ec)),
            ec);

    if(ec)
    {
        return std::nullopt;
    }

    public_app->listen(endpoint.value(), ec);

    if(ec)
    {
        return std::nullopt;
    }

    //cis_app

    init_cis_app(
            cis_app,
            *cis);

    endpoint = resolve_endpoint(
            ioc,
            *(config->get_entry<std::string>("cis_address", ec)),
            *(config->get_entry<uint16_t>("cis_port", ec)),
            ec);

    if(ec)
    {
        return std::nullopt;
    }

    cis_app->listen(endpoint.value());

    return std::optional<application>{
            std::in_place,
            private_constructor_delegate_t{},
            ioc,
            public_app,
            cis_app,
            std::move(config),
            std::move(db),
            std::move(cis),
            std::move(auth_manager_),
            std::move(rights_manager_),
            std::move(files),
            std::move(upload_handler),
            std::move(download_handler),
            std::move(webhooks_handler)};
}

application::private_constructor_delegate_t::private_constructor_delegate_t()
{}

application::application(
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
        std::unique_ptr<http::webhooks_handler> webhooks_handler)
    : ioc_(ioc)
    , app_(app)
    , cis_app_(cis_app)
    , config_(std::move(config))
    , db_(std::move(db))
    , cis_(std::move(cis))
    , auth_manager_(std::move(auth_manager_arg))
    , rights_manager_(std::move(rights_manager_arg))
    , files_(std::move(files))
    , upload_handler_(std::move(upload_handler))
    , download_handler_(std::move(download_handler))
    , webhooks_handler_(std::move(webhooks_handler))
{}

void application::run()
{
    ioc_.run();
}

void application::stop()
{
    ioc_.stop();
}
