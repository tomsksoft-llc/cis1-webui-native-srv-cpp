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
    http_handlers_chain.cpp
    cookie_parser.cpp
    auth_manager.cpp
    response.cpp
    file_handler.cpp
    multipart_form_handler.cpp
    file_util.cpp
    request_util.cpp
    cis_util.cpp
    child_process.cpp
    dirs.cpp
    init.cpp
    rights_manager.cpp
    http_handlers.cpp
    websocket_event_dispatcher.cpp
    websocket_event_handlers.cpp
    base_websocket_event_handler.cpp
    base_cis_srv_exception.cpp
    generic_error.cpp
    load_config_error.cpp)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)
add_executable(${PROJECT_NAME} main.cpp ${${PROJECT_NAME}_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
#set_property(TARGET ${PROJECT_NAME}
#             PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} stdc++fs Boost::regex Boost::filesystem)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
