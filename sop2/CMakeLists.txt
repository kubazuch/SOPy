cmake_minimum_required(VERSION 3.23)
project(SOP1 C)

set(CMAKE_C_STANDARD 99)

add_executable(lab1 lab1/lab1.c)
add_executable(lab2.s "lab2/kuchnia.c")
add_executable(lab2.c "lab2/klient.c")
add_executable(lab3.s "lab3/server.c")

add_executable(lab1.tut "lab1 tut/tut/prog22.c")
add_executable(lab1.exmpl "lab1 tut/example/prog.c")
add_executable(lab2.tut "lab2 tut/tut/mq_example.c")
add_executable(lab2.isi.s "lab2 tut/isi/server.c")
add_executable(lab2.isi.c "lab2 tut/isi/client.c")


target_link_libraries(lab2.tut rt)
target_link_libraries(lab2.isi.s rt)
target_link_libraries(lab2.isi.c rt)
target_link_libraries(lab2.s rt)
target_link_libraries(lab2.c rt)
target_link_libraries(lab3.s rt)
#target_link_libraries(lab2.c rt)

add_compile_options(-Wall -fsanitize=address,undefined -ansi -pedantic)