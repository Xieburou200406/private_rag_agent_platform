cmake_minimum_required(VERSION 3.16)
project(cpp_gateway VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -O2)

# 第一阶段：配置模块 + epoll-Reactor 基础框架（不含路由转发）
set(GATEWAY_SOURCES
    config.cpp
    epoll_server.cpp
    main.cpp
)

add_executable(gateway_server ${GATEWAY_SOURCES})

target_include_directories(gateway_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Linux 专有 epoll API
target_compile_definitions(gateway_server PRIVATE _GNU_SOURCE)

install(TARGETS gateway_server RUNTIME DESTINATION bin)
