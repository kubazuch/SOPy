cmake_minimum_required(VERSION 3.23)
project(SOP1 C)

set(CMAKE_C_STANDARD 99)

add_executable(lab0 lab0/lab0.c)
add_executable(lab1 lab1/lab1.c)
add_executable(lab2 lab2/lab2.c)
add_executable(lab2.popr "lab2 popr/lab2.c")
add_executable(lab3 lab3/lab3.c)
add_executable(lab4 lab4/lab4.c)

add_executable(lab1.example "lab1 tut/example/lister.c")
add_executable(lab1.cs lab1\ tut/cs/progrum.c)
add_executable(lab1.cs2 lab1\ tut/cs2/jugoslawiec.c)
add_executable(lab1.isi lab1\ tut/isi/libra.c)

add_executable(lab2.tut "lab2 tut/tut/prog16.c")
add_executable(lab2.example "lab2 tut/example/exmp.c")
add_executable(lab2.isi "lab2 tut/isi/prog.c")
add_executable(lab2.cs "lab2 tut/cs/lab.c")

add_executable(lab3.tut "lab3 tut/tut/sighand.c")
add_executable(lab3.cs "lab3 tut/cs/main.c")

add_executable(lab4.tut "lab4 tut/tut/prog.c")

add_compile_options(-Wall -fsanitize=address,undefined -ansi -pedantic)

target_link_libraries(lab3.tut m pthread)
target_link_libraries(lab3.cs m pthread)
target_link_libraries(lab4 m pthread rt)
target_link_libraries(lab4.tut m pthread rt)
target_link_libraries(lab4 m pthread rt)