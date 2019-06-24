#include "application.h"

#include <functional>

#include "http/error_handler.h"
#include "http/cookie_parser.h"
#include "http/common_handlers.h"
#include "http/url.h"
#include "websocket/event_dispatcher.h"
#include "websocket/event_handlers.h"
#include "cis/dirs.h"
#include "openssl_wrapper/openssl_wrapper.h"

using namespace std::placeholders;              // from <functional>

application::application(const init_params& params)
    : params_(params)
    , db_(params.db_root / "db.sqlite", params.admin)
    , ioc_{}
    , signals_(ioc_, SIGINT, SIGTERM)
    , cis_(ioc_, params_.cis_root, db_)
    , app_(std::make_shared<http::handlers_chain>())
    , cis_app_(std::make_shared<http::handlers_chain>())
    , auth_manager_(db_)
    , rights_manager_(db_)
    , files_(params.doc_root)
    , upload_handler_(
        std::filesystem::path{params.cis_root / cis::projects},
        rights_manager_)
    , download_handler_(
        std::filesystem::path{params.cis_root / cis::projects},
        rights_manager_)
    , webhooks_handler_(auth_manager_, rights_manager_, cis_)
{
    openssl::init();
    signals_.async_wait(
            [&](beast::error_code const&, int)
            {
                ioc_.stop();
            });
    init_app();
    init_cis_app();
}

void application::init_app()
{
    app_->set_error_handler(
            std::bind(
                    &http::error_handler::operator(),
                    std::make_shared<http::error_handler>(),
                    _1, _2, _3));
    app_->append_handler(&http::cookie_parser::parse);
    app_->append_handler(
            std::bind(
                    &http::handle_authenticate,
                    auth_manager_,
                    _1, _2, _3, _4));
    app_->append_handler(
            std::bind(
                    &http_router::operator(),
                    make_public_http_router(),
                    _1, _2, _3, _4));
    app_->append_ws_handler(
            std::bind(
                    &websocket_router::operator(),
                    make_ws_router(),
                    _1, _2, _3));

    app_->listen(ioc_, tcp::endpoint{params_.public_address, params_.public_port});
}

void application::init_cis_app()
{
    cis_app_->set_error_handler(
            std::bind(
                    &http::error_handler::operator(),
                    std::make_shared<http::error_handler>(),
                    _1, _2, _3));
    cis_app_->append_handler(
            std::bind(
                    &http_router::operator(),
                    make_cis_http_router(),
                    _1, _2,_3, _4));

    cis_app_->listen(ioc_, tcp::endpoint{params_.cis_address, params_.cis_port});
}

std::shared_ptr<http_router> application::make_public_http_router()
{
    auto router = std::make_shared<http_router>();

    router->add_route(
            url::make() / CT_STRING("upload") / url::bound_string() / url::string(),
            [&upload_handler = upload_handler_](auto&& ...args)
            {
                return upload_handler(std::forward<decltype(args)>(args)...);
            });

    router->add_route(
            url::make() / CT_STRING("download") / url::bound_string() / url::string(),
            [&download_handler = download_handler_](auto&& ...args)
            {
                return download_handler(std::forward<decltype(args)>(args)...);
            });

    auto webhooks_url = url::make() /
            CT_STRING("users") / url::bound_string() /
            CT_STRING("webhooks");

    router->add_route(
            webhooks_url / CT_STRING("github") /
                    url::bound_string() / url::bound_string() << url::query_string(),
            [&whh = webhooks_handler_](auto&& ...args)
            {
                return whh(
                        std::forward<decltype(args)>(args)...,
                        whh.api::github);
            });

    router->add_route(
            webhooks_url / CT_STRING("gitlab") /
                    url::bound_string() / url::bound_string() << url::query_string(),
            [&whh = webhooks_handler_](auto&& ...args)
            {
                return whh(
                        std::forward<decltype(args)>(args)...,
                        whh.api::gitlab);
            });

    std::function<http::handle_result(
            beast::http::request<beast::http::empty_body>&,
            request_context&,
            net::http_session::request_reader&,
            net::http_session::queue&)> cb = std::bind(
            &http::file_handler::operator(),
            files_,
            _1, _2, _3, _4);

    router->add_route(url::make() / CT_STRING("test_suite.html"), cb);
    router->add_route(url::make() / CT_STRING("static") / url::ignore(), cb);

    cb = std::bind(
            &http::file_handler::single_file,
            files_,
            _1, _2, _3, _4,
            "/index.html");
    router->add_route(url::make() / url::ignore(), cb);

    return router;
}

std::shared_ptr<websocket_router> application::make_ws_router()
{
    namespace ws = websocket;
    namespace wsh = ws::handlers;
    auto router = std::make_shared<websocket_router>();

    ws::event_dispatcher dispatcher;
    dispatcher.add_event_handler(ws::request_id::auth_login_pass,
            std::bind(
                    &wsh::authenticate,
                    std::ref(auth_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::auth_token,
            std::bind(&wsh::token,
                    std::ref(auth_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::logout,
            std::bind(&wsh::logout,
                    std::ref(auth_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::change_pass,
            std::bind(&wsh::change_pass,
                    std::ref(auth_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::list_users,
            std::bind(&wsh::list_users,
                std::ref(auth_manager_),
                std::ref(rights_manager_),
                _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::get_user_permissions,
            std::bind(&wsh::get_user_permissions,
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::set_user_permissions,
            std::bind(&wsh::set_user_permissions,
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::change_group,
            std::bind(&wsh::change_group,
                    std::ref(auth_manager_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::disable_user,
            std::bind(&wsh::disable_user,
                    std::ref(auth_manager_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::generate_api_key,
            std::bind(&wsh::generate_api_key,
                    std::ref(auth_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::get_api_key,
            std::bind(&wsh::get_api_key,
                    std::ref(auth_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::remove_api_key,
            std::bind(&wsh::remove_api_key,
                    std::ref(auth_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::list_projects,
            std::bind(&wsh::list_projects,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::get_project_info,
            std::bind(&wsh::get_project_info,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::get_job_info,
            std::bind(&wsh::get_job_info,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::run_job,
            std::bind(&wsh::run_job,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    std::ref(ioc_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::get_build_info,
            std::bind(&wsh::get_build_info,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::refresh_fs_entry,
            std::bind(&wsh::refresh_fs_entry,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::remove_fs_entry,
            std::bind(&wsh::remove_fs_entry,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::move_fs_entry,
            std::bind(&wsh::move_fs_entry,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::new_directory,
            std::bind(&wsh::new_directory,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::list_directory,
            std::bind(&wsh::list_directory,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::add_cis_cron,
            std::bind(&wsh::add_cis_cron,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));
    dispatcher.add_event_handler(ws::request_id::remove_cis_cron,
            std::bind(&wsh::remove_cis_cron,
                    std::ref(cis_),
                    std::ref(rights_manager_),
                    _1, _2, _3, _4));

    std::function <http::handle_result(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        tcp::socket& socket)> cb = [dispatcher](
                beast::http::request<beast::http::empty_body>& req,
                request_context& ctx,
                tcp::socket& socket)
                {
                    net::queued_websocket_session::accept_handler(
                            std::move(socket),
                            std::move(req),
                            std::bind(
                                &ws::event_dispatcher::dispatch,
                                dispatcher,
                                ctx,
                                _1, _2, _3, _4));
                    return http::handle_result::done;
                };
    router->add_route(url::make() / CT_STRING("ws"), cb);

    return router;
}

std::shared_ptr<http_router> application::make_cis_http_router()
{
    auto router = std::make_shared<http_router>();

    return router;
}

void application::run()
{
    ioc_.run();
}

void application::stop()
{
    ioc_.stop();
}
