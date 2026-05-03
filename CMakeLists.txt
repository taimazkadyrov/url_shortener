cmake_minimum_required(VERSION 3.15)
project(url_shortener)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
        src/main.cpp
        src/server/server.cpp
        src/server/router.cpp
        src/storage/redis_client.cpp
        src/generators/code_generator.cpp
        src/handlers/shorten_handler.cpp
        src/handlers/redirect_handler.cpp
        src/handlers/stats_handler.cpp
)

add_executable(url_shortener ${SOURCES})
find_package(hiredis REQUIRED)
target_link_libraries(url_shortener hiredis::hiredis)
target_include_directories(url_shortener PRIVATE src)