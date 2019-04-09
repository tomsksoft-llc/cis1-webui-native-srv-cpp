#pragma once

#include <string>
#include <optional>

#include "request_context.h"
#include "net/queued_websocket_session.h"
#include "auth_manager.h"
#include "rights_manager.h"
#include "cis_util.h"

#include <rapidjson/document.h>

namespace websocket
{

namespace handlers
{

//users

std::optional<std::string> authenticate(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> token(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> logout(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> change_pass(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> list_users(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> get_user_permissions(
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> set_user_permissions(
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> change_group(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> disable_user(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> generate_api_key(
        const std::shared_ptr<auth_manager>& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

//cis

std::optional<std::string> list_projects(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> get_project_info(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> get_job_info(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> run_job(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        boost::asio::io_context& io_ctx,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> rename_job(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> get_build_info(
        const std::shared_ptr<project_list>& projects,
        const std::shared_ptr<rights_manager>& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

} // namespace handlers

} // namespace websocket
