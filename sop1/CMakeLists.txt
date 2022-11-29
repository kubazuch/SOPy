cmake_minimum_required(VERSION 3.23)
project(SOP1 C)

set(CMAKE_C_STANDARD 99)

add_executable(lab0 lab0/lab0.c)
add_executable(lab1 lab1/lab1.c)

add_executable(lab1.example "lab1 tut/example/lister.c")
add_executable(lab1.cs lab1\ tut/cs/progrum.c)
add_executable(lab1.cs2 lab1\ tut/cs2/jugoslawiec.c)
add_executable(lab1.isi lab1\ tut/isi/libra.c)
add_compile_options(-Wall -fsanitize=address,undefined -ansi -pedantic)