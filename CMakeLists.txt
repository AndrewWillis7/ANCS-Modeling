cmake_minimum_required(VERSION 3.16)
project(anc_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(anc_core
    core/src/LMSfilter.c
    core/src/anc_runner.c
)

target_include_directories(anc_core PUBLIC
    ${CMAKE_SOURCE_DIR}/core/include
)

add_executable(anc_cli
    core/src/main.c
)

target_link_libraries(anc_cli anc_core)