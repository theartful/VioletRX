find_package(Gnuradio REQUIRED COMPONENTS analog audio blocks digital filter fft network)
find_package(Gnuradio-osmosdr REQUIRED)

add_library(
core
    receivers/receiver_base.h
    receivers/receiver_base.cpp
    receivers/nbrx.h
    receivers/nbrx.cpp
    receivers/wfmrx.h
    receivers/wfmrx.cpp
    interfaces/udp_sink_f.h
    interfaces/udp_sink_f.cpp
    receiver.h
    receiver.cpp
    vfo_channel.h
    vfo_channel.cpp
)

target_include_directories(
core
    PUBLIC "${SOURCE_DIRECTORY}"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
core
PUBLIC
    ${GNURADIO_OSMOSDR_LIBRARIES}
    gnuradio::gnuradio-network
    dsp

PRIVATE
    spdlog::spdlog
)

target_include_directories(core PUBLIC "${GNURADIO_OSMOSDR_INCLUDE_DIRS}")
