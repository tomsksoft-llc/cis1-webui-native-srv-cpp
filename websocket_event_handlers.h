#pragma once

#include <string>
#include <optional>

#include "request_context.h"
#include "net/queued_websocket_session.h"
#include "auth_manager.h"
#include "rights_manager.h"
#include "cis_util.h"

#include <rapidjson/document.h>

//users

std::optional<std::string> ws_handle_authenticate(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_token(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_logout(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_change_pass(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_list_users(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_get_user_permissions(
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_set_user_permissions(
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_change_group(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_disable_user(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_generate_api_key(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

//cis

std::optional<std::string> ws_handle_list_projects(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_list_jobs(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_get_job_info(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_run_job(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        boost::asio::io_context& io_ctx,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_rename_job(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> ws_handle_get_build_info(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);
