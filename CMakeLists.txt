cmake_minimum_required(VERSION 3.9)

project(ts_ws)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(Threads REQUIRED)
find_package(Filesystem REQUIRED Final)

set(${PROJECT_NAME}_SRC

    net/basic_websocket_session.cpp
    net/http_session.cpp
    net/socket_util.cpp
    net/queued_websocket_session.cpp
    net/websocket_queue.cpp
    net/listener.cpp
    net/fail.cpp

    websocket/event_dispatcher.cpp
    websocket/event_handlers.cpp
    websocket/transaction.cpp
    websocket/dto/dto_to_event_id.cpp

    tpl_reflect/json_engine.cpp

    openssl_wrapper/openssl_wrapper.cpp

    beast_ext/knuth_morris_pratt_searcher.cpp
    beast_ext/message_stream_parser.cpp
    beast_ext/multipart_stream_parser.cpp

    cis/dirs.cpp
    cis/cis_structs.cpp
    cis/fs_mapper.cpp
    cis/cis_manager.cpp

    exceptions/base_cis_srv_exception.cpp

    http/request_util.cpp
    http/response.cpp
    http/file_handler.cpp
    http/common_handlers.cpp
    http/cookie_parser.cpp
    http/multipart_form_handler.cpp
    http/download_handler.cpp
    http/webhooks_handler.cpp
    http/handlers_chain.cpp
    http/error_handler.cpp

    application.cpp
    bound_task_chain.cpp
    base64.cpp
    cron_utils.cpp
    random_generator.cpp
    init.cpp
    auth_manager.cpp
    database.cpp
    file_util.cpp
    rights_manager.cpp
    child_process.cpp
    main.cpp)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(
    ${PROJECT_NAME}
    Threads::Threads
    std::filesystem
    ${CONAN_LIBS})

