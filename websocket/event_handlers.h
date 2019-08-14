#pragma once

#include <string>
#include <optional>

#include "request_context.h"
#include "auth_manager.h"
#include "rights_manager.h"
#include "cis/cis_manager.h"
#include "transaction.h"

#include "websocket/dto/auth_login_pass.h"
#include "websocket/dto/auth_token.h"
#include "websocket/dto/auth_logout.h"
#include "websocket/dto/user_auth_change_pass.h"
#include "websocket/dto/user_list.h"
#include "websocket/dto/user_permissions_get.h"
#include "websocket/dto/user_permissions_set.h"
#include "websocket/dto/user_auth_change_group.h"
#include "websocket/dto/user_auth_ban.h"
#include "websocket/dto/user_api_key_generate.h"
#include "websocket/dto/user_api_key_get.h"
#include "websocket/dto/user_api_key_remove.h"
#include "websocket/dto/cis_project_list_get.h"
#include "websocket/dto/cis_project_info.h"
#include "websocket/dto/cis_job_info.h"
#include "websocket/dto/cis_job_run.h"
#include "websocket/dto/cis_build_info.h"
#include "websocket/dto/fs_entry_refresh.h"
#include "websocket/dto/fs_entry_remove.h"
#include "websocket/dto/fs_entry_move.h"
#include "websocket/dto/fs_entry_new_dir.h"
#include "websocket/dto/fs_entry_list.h"
#include "websocket/dto/cis_cron_add.h"
#include "websocket/dto/cis_cron_remove.h"
#include "websocket/dto/cis_cron_list.h"

namespace websocket
{

namespace handlers
{

//users

void authenticate(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_login_pass& req,
        transaction tr);

void token(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_token& req,
        transaction tr);

void logout(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_logout& req,
        transaction tr);

void change_pass(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::user_auth_change_pass& req,
        transaction tr);

void list_users(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::user_list& req,
        transaction tr);

void get_user_permissions(
        rights_manager& rights,
        request_context& ctx,
        const dto::user_permissions_get& req,
        transaction tr);

void set_user_permissions(
        rights_manager& rights,
        request_context& ctx,
        const dto::user_permissions_set& req,
        transaction tr);

void change_group(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::user_auth_change_group& req,
        transaction tr);

void disable_user(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::user_auth_ban& req,
        transaction tr);

void generate_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::user_api_key_generate& req,
        transaction tr);

void get_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::user_api_key_get& req,
        transaction tr);

void remove_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::user_api_key_remove& req,
        transaction tr);

//cis

void list_projects(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_project_list_get& req,
        transaction tr);

void get_project_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_project_info& req,
        transaction tr);

void get_job_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_job_info& req,
        transaction tr);

void run_job(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_job_run& req,
        transaction tr);

void get_build_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_build_info& req,
        transaction tr);

void refresh_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_refresh& req,
        transaction tr);

void remove_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_remove& req,
        transaction tr);

void move_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_move& req,
        transaction tr);

void new_directory(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_new_dir& req,
        transaction tr);

void list_directory(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_list& req,
        transaction tr);

void add_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_cron_add& req,
        transaction tr);

void remove_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_cron_remove& req,
        transaction tr);

void list_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_cron_list& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
