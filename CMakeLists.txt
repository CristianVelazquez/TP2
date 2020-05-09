cmake_minimum_required(VERSION 3.15)
project(TP2 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic -Wextra -Wconversion")
add_executable(mainBmp mainBmp.c)
add_executable(ejemplo ejemplo.c)
target_link_libraries(ejemplo -lm)
target_link_libraries(mainBmp -lm)

