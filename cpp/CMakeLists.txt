find_package(spdlog REQUIRED)

set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

add_subdirectory(utility)
add_subdirectory(dsp)
add_subdirectory(core)
add_subdirectory(async_core)
add_subdirectory(async_core_c)
add_subdirectory(grpc_server)
add_subdirectory(qtclient)
