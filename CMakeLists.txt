cmake_minimum_required(VERSION 3.9)

project(ts_ws)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(Threads REQUIRED)
find_package(Filesystem REQUIRED Final)

set(${PROJECT_NAME}_SRC

    src/net/basic_websocket_session.cpp
    src/net/http_session.cpp
    src/net/socket_util.cpp
    src/net/queued_websocket_session.cpp
    src/net/websocket_queue.cpp
    src/net/listener.cpp
    src/net/fail.cpp

    src/websocket/event_dispatcher.cpp
    src/websocket/transaction.cpp

    src/websocket/handlers/add_cis_cron.cpp
    src/websocket/handlers/authenticate.cpp
    src/websocket/handlers/ban_user.cpp
    src/websocket/handlers/change_group.cpp
    src/websocket/handlers/change_pass.cpp
    src/websocket/handlers/generate_api_key.cpp
    src/websocket/handlers/get_api_key.cpp
    src/websocket/handlers/get_build_info.cpp
    src/websocket/handlers/get_job_info.cpp
    src/websocket/handlers/get_project_info.cpp
    src/websocket/handlers/get_user_permissions.cpp
    src/websocket/handlers/list_cis_cron.cpp
    src/websocket/handlers/list_directory.cpp
    src/websocket/handlers/list_projects.cpp
    src/websocket/handlers/list_users.cpp
    src/websocket/handlers/logout.cpp
    src/websocket/handlers/move_fs_entry.cpp
    src/websocket/handlers/new_directory.cpp
    src/websocket/handlers/refresh_fs_entry.cpp
    src/websocket/handlers/remove_api_key.cpp
    src/websocket/handlers/remove_cis_cron.cpp
    src/websocket/handlers/remove_fs_entry.cpp
    src/websocket/handlers/run_job.cpp
    src/websocket/handlers/set_user_permissions.cpp
    src/websocket/handlers/token.cpp
    src/websocket/handlers/unban_user.cpp

    src/tpl_reflect/json_engine.cpp

    src/openssl_wrapper/openssl_wrapper.cpp

    src/beast_ext/knuth_morris_pratt_searcher.cpp
    src/beast_ext/message_stream_parser.cpp
    src/beast_ext/multipart_stream_parser.cpp

    src/cis/dirs.cpp
    src/cis/cis_structs.cpp
    src/cis/fs_mapper.cpp
    src/cis/cis_manager.cpp

    src/exceptions/base_cis_srv_exception.cpp

    src/http/request_util.cpp
    src/http/response.cpp
    src/http/file_handler.cpp
    src/http/common_handlers.cpp
    src/http/cookie_parser.cpp
    src/http/multipart_form_handler.cpp
    src/http/download_handler.cpp
    src/http/webhooks_handler.cpp
    src/http/handlers_chain.cpp
    src/http/error_handler.cpp

    src/application.cpp
    src/bound_task_chain.cpp
    src/base64.cpp
    src/path_utils.cpp
    src/cron_utils.cpp
    src/random_generator.cpp
    src/init.cpp
    src/auth_manager.cpp
    src/database.cpp
    src/file_util.cpp
    src/rights_manager.cpp
    src/child_process.cpp
    src/main.cpp)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(
    ${PROJECT_NAME}
    Threads::Threads
    std::filesystem
    ${CONAN_LIBS})

