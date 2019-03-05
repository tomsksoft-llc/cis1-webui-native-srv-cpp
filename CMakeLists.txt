cmake_minimum_required(VERSION 3.9)

project(ts_ws)

find_package(Threads REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS regex filesystem REQUIRED)

set(${PROJECT_NAME}_SRC
    fail.cpp
    http_session.cpp
    listener.cpp
    basic_websocket_session.cpp
    queued_websocket_session.cpp
    router.cpp
    publisher.cpp
    file_handler.cpp
    login_handler.cpp
    projects_handler.cpp
    socket_util.cpp
    file_util.cpp
    request_util.cpp
    cis_util.cpp)

add_executable(${PROJECT_NAME} main.cpp ${${PROJECT_NAME}_SRC})
#set_property(TARGET ${PROJECT_NAME}
#             PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} stdc++fs Boost::regex Boost::filesystem)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
