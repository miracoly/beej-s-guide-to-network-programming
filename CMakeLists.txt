cmake_minimum_required(VERSION 3.31)
project(beejs_guide_to_c_programming LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(showip
        src/showip.c
)

add_executable(simple_server
        src/simple_server.c
)

add_executable(simple_client
        src/simple_client.c
)
