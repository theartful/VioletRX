set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

find_package(spdlog REQUIRED)

include(FetchContent)
FetchContent_Declare(
  broadcast_queue
  GIT_REPOSITORY    https://github.com/theartful/broadcast_queue
  GIT_TAG           master
)
FetchContent_MakeAvailable(broadcast_queue)

include(FetchContent)
FetchContent_Declare(
    function2
    GIT_REPOSITORY    https://github.com/Naios/function2
    GIT_TAG           master
)
FetchContent_MakeAvailable(function2)

FetchContent_Declare(
  concurrentqueue
  GIT_REPOSITORY    https://github.com/cameron314/concurrentqueue
  GIT_TAG           master
)
FetchContent_MakeAvailable(concurrentqueue)

add_subdirectory(utility)
add_subdirectory(dsp)
add_subdirectory(core)
add_subdirectory(async_core)
add_subdirectory(async_core_c)
add_subdirectory(grpc)
add_subdirectory(qtclient)
