cmake_minimum_required(VERSION 4.3.1)
set(CMAKE_CXX_COMPILER gcc)
project(shellify VERSION 0.1.0 LANGUAGES C CXX)

add_executable(shellify
    src/main.c
    src/shellify.c
    src/buffer.c
    src/input.c
)

