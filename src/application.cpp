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

using namespace std::placeholders;              // from <functional>

application::application(const init_params& params)
    : params_(params)
    , db_(params.db_root / "db.sqlite", params.admin)
    , ioc_{}
    , signals_(ioc_, SIGINT, SIGTERM)
    , cis_(ioc_, params_.cis_root, params_.cis_address, params_.cis_port, db_)
    , app_(std::make_shared<http::handlers_chain>())
    , cis_app_(ioc_)
    , auth_manager_(ioc_, db_)
    , rights_manager_(db_)
    , files_(params.doc_root.string())
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
                    std::ref(auth_manager_),
                    _1, _2, _3, _4));
    app_->append_handler(
            std::bind(
                    &http_router::operator(),
                    make_http_router(),
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
    struct session_context
    {
        std::shared_ptr<cis::session> session = nullptr;
    };

    cis1::proto_utils::event_dispatcher<session_context&> dispatcher;

    dispatcher.add_event_handler<cis1::cwu::session_auth>(
            [&](     session_context& ctx,
                    const cis1::cwu::session_auth& dto,
                    cis1::proto_utils::transaction tr)
            {
                ctx.session = cis_.connect_to_session(dto.session_id);

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

    cis_app_.set_session_acceptor(
            [dispatcher]() mutable
            {
                return [&dispatcher, ctx = session_context{}](
                            boost::asio::const_buffer buffer,
                            std::shared_ptr<cis1::proto_utils::queue_interface> queue) mutable
                        {
                            dispatcher.dispatch(ctx, true, buffer, buffer.size(), queue);
                        };
            });

    cis_app_.listen({params_.cis_address, params_.cis_port});
}

std::shared_ptr<http_router> application::make_http_router()
{
    auto router = std::make_shared<http_router>();

    router->add_route(
            url::make() / CT_STRING("upload") / url::bound_string() / url::string(),
            [&upload_handler = upload_handler_](auto&& ...args)
            {
                return upload_handler.upload(std::forward<decltype(args)>(args)...);
            });

    router->add_route(
            url::make() / CT_STRING("upload") / url::bound_string(),
            [&upload_handler = upload_handler_](auto&& ...args)
            {
                return upload_handler.upload(std::forward<decltype(args)>(args)...);
            });

    router->add_route(
            url::make() / CT_STRING("replace") / url::bound_string() / url::string(),
            [&upload_handler = upload_handler_](auto&& ...args)
            {
                return upload_handler.replace(std::forward<decltype(args)>(args)...);
            });

    router->add_route(
            url::make() / CT_STRING("replace") / url::bound_string(),
            [&upload_handler = upload_handler_](auto&& ...args)
            {
                return upload_handler.replace(std::forward<decltype(args)>(args)...);
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
                        http::webhooks_handler::api::github);
            });

    router->add_route(
            webhooks_url / CT_STRING("gitlab") /
                    url::bound_string() / url::bound_string() << url::query_string(),
            [&whh = webhooks_handler_](auto&& ...args)
            {
                return whh(
                        std::forward<decltype(args)>(args)...,
                        http::webhooks_handler::api::gitlab);
            });

    router->add_route(
            webhooks_url / CT_STRING("plain") /
                    url::bound_string() / url::bound_string() << url::query_string(),
            [&whh = webhooks_handler_](auto&& ...args)
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
            files_,
            _1, _2, _3, _4,
            "/index.html");

    router->add_route(url::root(), cb);

    router->add_route(url::make() / url::ignore(),
            [&files = files_](auto&& ...args)
            {
                return files(std::forward<decltype(args)>(args)...);
            });

    return router;
}

std::shared_ptr<websocket_router> application::make_ws_router()
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
    dispatcher.add_event_handler<ws::dto::user_list>(
            std::bind(&wsh::list_users,
                std::ref(auth_manager_),
                std::ref(rights_manager_),
                _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_permissions_get>(
            std::bind(&wsh::get_user_permissions,
                    std::ref(rights_manager_),
                    _1, _2, _3));
    dispatcher.add_event_handler<ws::dto::user_permissions_set>(
            std::bind(&wsh::set_user_permissions,
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
    dispatcher.add_event_handler<ws::dto::cis_job_add>(
            std::bind(&wsh::add_cis_job,
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
    dispatcher.add_event_handler<ws::dto::cis_cron_add>(
            [this](auto&& ...args){
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
            [this](auto&& ...args){
                    wsh::session_subscribe(
                            cis_,
                            std::forward<decltype(args)>(args)...);
            });
    dispatcher.add_event_handler<ws::dto::cis_session_unsubscribe>(
            [this](auto&& ...args){
                    wsh::session_unsubscribe(
                            cis_,
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

void application::run()
{
    ioc_.run();
}

void application::stop()
{
    ioc_.stop();
}
