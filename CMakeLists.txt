cmake_minimum_required(VERSION 3.16)

# 项目名称，语言
project(SnakeGame LANGUAGES CXX)

# 设置C++标准 C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# 添加可执行程序：名字 SnakeGame，源码文件
add_executable(SnakeGame src/main.cpp)

# Linux ncurses依赖（Windows不需要）
if(UNIX)
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIR})
    target_link_libraries(SnakeGame ${CURSES_LIBRARIES})
endif()
