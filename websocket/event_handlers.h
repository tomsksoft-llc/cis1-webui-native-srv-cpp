#pragma once

#include <string>
#include <optional>

#include "request_context.h"
#include "net/queued_websocket_session.h"
#include "auth_manager.h"
#include "rights_manager.h"
#include "cis/project_list.h"
#include "cis/job.h"

#include <rapidjson/document.h>

namespace websocket
{

namespace handlers
{

//users

std::optional<std::string> authenticate(
        auth_manager& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> token(
        auth_manager& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> logout(
        auth_manager& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> change_pass(
        auth_manager& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> list_users(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> get_user_permissions(
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> set_user_permissions(
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> change_group(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> disable_user(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> generate_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

//cis

std::optional<std::string> list_projects(
        cis::project_list& projects,
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> get_project_info(
        cis::project_list& projects,
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> get_job_info(
        cis::project_list& projects,
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> run_job(
        cis::project_list& projects,
        rights_manager& rights,
        boost::asio::io_context& io_ctx,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> rename_job(
        cis::project_list& projects,
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

std::optional<std::string> get_build_info(
        cis::project_list& projects,
        rights_manager& rights,
        request_context& ctx,
        const rapidjson::Value& request_data,
        rapidjson::Value& response_data,
        rapidjson::Document::AllocatorType& allocator);

} // namespace handlers

} // namespace websocket
