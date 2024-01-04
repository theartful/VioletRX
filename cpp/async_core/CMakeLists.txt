# for boost signals
find_package(Boost REQUIRED COMPONENTS system)

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

add_library(
async_core_iface
    async_receiver_iface.h
    async_vfo_iface.h
    error_codes.h
    error_codes.cpp
    events.h
)

target_link_libraries(async_core_iface INTERFACE function2 Boost::system)
target_include_directories(async_core_iface INTERFACE "${SOURCE_DIRECTORY}")


add_library(
async_core
    worker_thread.h
    worker_thread.cpp
    async_receiver.h
    async_receiver.cpp
    async_vfo.h
    async_vfo.cpp
)

target_link_libraries(
async_core
PUBLIC
    async_core_iface
PRIVATE
    core
    concurrentqueue
    utility
)
