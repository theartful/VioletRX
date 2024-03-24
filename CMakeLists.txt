cmake_minimum_required(VERSION 3.15)
project(violetrx)

# enable extra compiler warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /WX)
endif()

set(SOURCE_DIRECTORY "${PROJECT_SOURCE_DIR}/cpp")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")

add_subdirectory(cpp)
# add_subdirectory(rust)
