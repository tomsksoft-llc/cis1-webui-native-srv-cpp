cmake_minimum_required(VERSION 3.9)

project(ts_ws)

find_package(Threads REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS regex filesystem REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

aux_source_directory(beast_ext ${PROJECT_NAME}_BEAST_EXT_SRC)
aux_source_directory(exceptions ${PROJECT_NAME}_EXCEPTIONS_SRC)
aux_source_directory(cis ${PROJECT_NAME}_CIS_SRC)
aux_source_directory(http ${PROJECT_NAME}_HTTP_SRC)
aux_source_directory(net ${PROJECT_NAME}_NET_SRC)
aux_source_directory(sqlite_orm_ext ${PROJECT_NAME}_SQLITE_ORM_EXT_SRC)
aux_source_directory(websocket ${PROJECT_NAME}_WEBSOCKET_SRC)
aux_source_directory(openssl_wrapper ${PROJECT_NAME}_OPENSSL_WRAPPER_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_NAME}_SRC)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)
add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_BEAST_EXT_SRC}
    ${${PROJECT_NAME}_EXCEPTIONS_SRC}
    ${${PROJECT_NAME}_CIS_SRC}
    ${${PROJECT_NAME}_HTTP_SRC}
    ${${PROJECT_NAME}_NET_SRC}
    ${${PROJECT_NAME}_SQLITE_ORM_EXT_SRC}
    ${${PROJECT_NAME}_WEBSOCKET_SRC}
    ${${PROJECT_NAME}_OPENSSL_WRAPPER_SRC}
    ${${PROJECT_NAME}_SRC})

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
