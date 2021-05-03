cmake_minimum_required(VERSION 3.17)
project(spo_lab1 C)

set(CMAKE_C_STANDARD 11)

add_executable(spo_lab1 main.c part1.c part2.c utils.c xfs_operations.c)
add_library(spo_lab2 SHARED main.c part1.c part2.c utils.c xfs_operations.c)


target_include_directories(spo_lab2 PRIVATE includes)
#set_target_properties(spo_lab1_lib PROPERTIES PUBLIC_HEADER "includes/include_main.h;includes/include.h")

