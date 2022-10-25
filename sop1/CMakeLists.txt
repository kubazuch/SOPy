cmake_minimum_required(VERSION 3.23)
project(lab0/lab0 C)

set(CMAKE_C_STANDARD 11)

add_executable(lab0 lab0/lab0.c)
add_executable(lab1 lab1/lab1.c)
add_compile_options(-Wall -fsanitize=address,undefined -ansi -pedantic)