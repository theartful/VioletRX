# for boost signals
find_package(Boost REQUIRED COMPONENTS system)

add_library(
async_core_iface
    async_receiver_iface.h
    async_vfo_iface.h
    error_codes.h
    error_codes.cpp
    events.h
    events_format.h
)

target_link_libraries(async_core_iface PUBLIC function2 Boost::system)
target_include_directories(async_core_iface PUBLIC "${SOURCE_DIRECTORY}")

add_library(
async_core
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
