cmake_minimum_required(VERSION 3.23)
project(SOP1 C)

set(CMAKE_C_STANDARD 99)

#add_executable(lab0 lab0/lab0.c)

add_executable(lab1.tut "lab1 tut/tut/prog22.c")
add_executable(lab1.exmpl "lab1 tut/example/prog.c")

add_compile_options(-Wall -fsanitize=address,undefined -ansi -pedantic)