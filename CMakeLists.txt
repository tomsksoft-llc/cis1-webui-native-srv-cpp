cmake_minimum_required(VERSION 3.9)

project(ts_ws)

find_package(Threads REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS regex filesystem REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

set(${PROJECT_NAME}_SRC

    net/basic_websocket_session.cpp
    net/http_session.cpp
    net/socket_util.cpp
    net/queued_websocket_session.cpp
    net/listener.cpp
    net/fail.cpp

    websocket/event_dispatcher.cpp
    websocket/base_event_handler.cpp
    websocket/event_handlers.cpp

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
    http/webhooks_handler.cpp
    http/handlers_chain.cpp
    http/error_handler.cpp

    application.cpp
    random_generator.cpp
    init.cpp
    auth_manager.cpp
    database.cpp
    file_util.cpp
    rights_manager.cpp
    child_process.cpp
    main.cpp)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_include_directories(
    ${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(
    ${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite_orm/include>)
target_include_directories(
    ${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${OPENSSL_INCLUDE_DIR})

target_link_libraries(
    ${PROJECT_NAME}
    ${CMAKE_THREAD_LIBS_INIT}
    stdc++fs
    Boost::filesystem
    Boost::regex
    sqlite3
    OpenSSL::Crypto)
