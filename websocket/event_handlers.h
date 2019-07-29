#pragma once

#include <string>
#include <optional>

#include "request_context.h"
#include "auth_manager.h"
#include "rights_manager.h"
#include "cis/cis_manager.h"
#include "transaction.h"

#include "websocket/dto/auth_login_pass_request.h"
#include "websocket/dto/auth_token_request.h"
#include "websocket/dto/logout_request.h"
#include "websocket/dto/change_pass_request.h"
#include "websocket/dto/get_user_list_request.h"
#include "websocket/dto/get_user_permissions_request.h"
#include "websocket/dto/set_user_permissions_request.h"
#include "websocket/dto/change_group_request.h"
#include "websocket/dto/disable_user_request.h"
#include "websocket/dto/generate_api_key_request.h"
#include "websocket/dto/get_api_key_request.h"
#include "websocket/dto/remove_api_key_request.h"
#include "websocket/dto/get_project_list_request.h"
#include "websocket/dto/get_project_info_request.h"
#include "websocket/dto/get_job_info_request.h"
#include "websocket/dto/run_job_request.h"
#include "websocket/dto/get_build_info_request.h"
#include "websocket/dto/refresh_fs_entry_request.h"
#include "websocket/dto/remove_fs_entry_request.h"
#include "websocket/dto/move_fs_entry_request.h"
#include "websocket/dto/new_directory_request.h"
#include "websocket/dto/list_directory_request.h"
#include "websocket/dto/add_cis_cron_request.h"
#include "websocket/dto/remove_cis_cron_request.h"
#include "websocket/dto/list_cis_cron_request.h"

namespace websocket
{

namespace handlers
{

//users

void authenticate(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_login_pass_request& req,
        transaction tr);

void token(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_token_request& req,
        transaction tr);

void logout(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::logout_request& req,
        transaction tr);

void change_pass(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::change_pass_request& req,
        transaction tr);

void list_users(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::get_user_list_request& req,
        transaction tr);

void get_user_permissions(
        rights_manager& rights,
        request_context& ctx,
        const dto::get_user_permissions_request& req,
        transaction tr);

void set_user_permissions(
        rights_manager& rights,
        request_context& ctx,
        const dto::set_user_permissions_request& req,
        transaction tr);

void change_group(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::change_group_request& req,
        transaction tr);

void disable_user(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::disable_user_request& req,
        transaction tr);

void generate_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::generate_api_key_request& req,
        transaction tr);

void get_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::get_api_key_request& req,
        transaction tr);

void remove_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::remove_api_key_request& req,
        transaction tr);

//cis

void list_projects(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::get_project_list_request& req,
        transaction tr);

void get_project_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::get_project_info_request& req,
        transaction tr);

void get_job_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::get_job_info_request& req,
        transaction tr);

void run_job(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::run_job_request& req,
        transaction tr);

void get_build_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::get_build_info_request& req,
        transaction tr);

void refresh_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::refresh_fs_entry_request& req,
        transaction tr);

void remove_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::remove_fs_entry_request& req,
        transaction tr);

void move_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::move_fs_entry_request& req,
        transaction tr);

void new_directory(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::new_directory_request& req,
        transaction tr);

void list_directory(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::list_directory_request& req,
        transaction tr);

void add_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::add_cis_cron_request& req,
        transaction tr);

void remove_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::remove_cis_cron_request& req,
        transaction tr);

void list_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::list_cis_cron_request& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
