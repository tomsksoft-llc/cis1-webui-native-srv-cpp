cmake_minimum_required(VERSION 3.9)

project(ts_ws)

find_package(Threads REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS regex filesystem REQUIRED)
find_package(RapidJSON REQUIRED)

set(${PROJECT_NAME}_SRC
    net/fail.cpp
    net/http_session.cpp
    net/listener.cpp
    net/basic_websocket_session.cpp
    net/queued_websocket_session.cpp
    net/socket_util.cpp
    cis/dirs.cpp
    cis/job.cpp
    cis/project_list.cpp
    exceptions/base_cis_srv_exception.cpp
    exceptions/generic_error.cpp
    exceptions/load_config_error.cpp
    http/handlers_chain.cpp
    http/error_handler.cpp
    http/cookie_parser.cpp
    http/response.cpp
    http/file_handler.cpp
    http/multipart_form_handler.cpp
    http/request_util.cpp
    http/common_handlers.cpp
    websocket/event_dispatcher.cpp
    websocket/event_handlers.cpp
    websocket/base_event_handler.cpp
    auth_manager.cpp
    file_util.cpp
    child_process.cpp
    db_dirs.cpp
    init.cpp
    rights_manager.cpp
    application.cpp)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)
add_executable(${PROJECT_NAME} main.cpp ${${PROJECT_NAME}_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
#set_property(TARGET ${PROJECT_NAME}
#             PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} stdc++fs Boost::regex Boost::filesystem)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
