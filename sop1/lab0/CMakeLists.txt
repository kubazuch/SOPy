cmake_minimum_required(VERSION 3.23)
project(lab0 C)

set(CMAKE_C_STANDARD 11)

add_executable(lab0 main.c)
add_compile_options(-Wall -fsanitize=address,undefined)