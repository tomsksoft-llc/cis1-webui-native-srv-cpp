cmake_minimum_required(VERSION 3.9)

project(ts_ws)

find_package(Threads REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS regex filesystem REQUIRED)

set(${PROJECT_NAME}_SRC
    fail.cpp
    net/http_session.cpp
    net/listener.cpp
    net/basic_websocket_session.cpp
    net/queued_websocket_session.cpp
    net/router.cpp
    net/socket_util.cpp
    response.cpp
    publisher.cpp
    file_handler.cpp
    login_handler.cpp
    projects_handler.cpp
    file_util.cpp
    request_util.cpp
    cis_util.cpp
    child_process.cpp
    cis_dirs.cpp
    init.cpp
    rights_manager.cpp)

add_executable(${PROJECT_NAME} main.cpp ${${PROJECT_NAME}_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
#set_property(TARGET ${PROJECT_NAME}
#             PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} stdc++fs Boost::regex Boost::filesystem)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
