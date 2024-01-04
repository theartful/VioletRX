add_library(
async_core_c
SHARED
    async_receiver_c.h
    async_receiver_c.cpp
    async_vfo_c.h
    async_vfo_c.cpp
    events_c.h
    events_conversion.h
    events_conversion.cpp
)

target_link_libraries(
async_core_c
PRIVATE
    async_core
)
