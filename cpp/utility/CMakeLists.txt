add_library(
utility
    assert.h
    worker_thread.h
    worker_thread.cpp
)

target_link_libraries(
utility
PUBLIC
    spdlog::spdlog
    concurrentqueue
    function2
)
