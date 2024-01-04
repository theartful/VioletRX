#ifndef ASYNC_CORE_TYPES_H
#define ASYNC_CORE_TYPES_H

#include <bits/chrono.h>
#include <chrono>
#include <cstdint>
#include <string>
#include <type_traits>

#include <boost/signals2.hpp>
#include <function2/function2.hpp>

#include "async_core/error_codes.h"

namespace core
{

class AsyncVfoIface;
using AsyncVfoIfaceSptr = std::shared_ptr<AsyncVfoIface>;

class AsyncReceiverIface;
using AsyncReceiverIfaceSptr = std::shared_ptr<AsyncReceiverIface>;

using Connection = boost::signals2::connection;

template <typename... Args>
using Signal = boost::signals2::signal<Args...>;

struct Timestamp {
    uint64_t seconds;
    uint32_t nanos;
};

struct GainStage {
    std::string name;
    double start;
    double stop;
    double step;
    double value;
};

struct FilterRange {
    int64_t lowMin;
    int64_t lowMax;
    int64_t highMin;
    int64_t highMax;
    bool symmetric;
};

enum class FilterShape {
    SOFT = 0,   /*!< Soft: Transition band is TBD of width */
    NORMAL = 1, /*!< Normal: Transition band is TBD of width */
    SHARP = 2   /*!< Sharp: Transition band is TBD of width */
};

struct Filter {
    FilterShape shape;
    int32_t low;
    int32_t high;
};

struct UdpStreamParams {
    std::string host;
    int port;
    bool stereo;
};

struct SnifferParams {
    int sampleRate;
    int buffSize;
};

enum class Demod {
    OFF = 0,              /*!< Demodulator completely off. */
    RAW = 1,              /*!< Raw I/Q passthrough. */
    AM = 2,               /*!< Amplitude modulation. */
    AM_SYNC = 3,          /*!< Amplitude modulation (synchronous demod). */
    LSB = 4,              /*!< Lower side band. */
    USB = 5,              /*!< Upper side band. */
    CWL = 6,              /*!< CW using LSB filter. */
    CWU = 7,              /*!< CW using USB filter. */
    NFM = 8,              /*!< Narrow band FM. */
    WFM_MONO = 9,         /*!< Broadcast FM (mono). */
    WFM_STEREO = 10,      /*!< Broadcast FM (stereo). */
    WFM_STEREO_OIRT = 11, /*!< Broadcast FM (stereo oirt). */
    LAST = 12
};

// see gr::fft::window
enum class WindowType {
    HAMMING = 0,
    HANN = 1,
    BLACKMAN = 2,
    RECTANGULAR = 3,
    KAISER = 4,
    BLACKMAN_HARRIS = 5,
    BARTLETT = 6,
    FLATTOP = 7,
    NUTTALL = 8,
    BLACKMAN_NUTTALL = 8,
    NUTTALL_CFD = 9,
    WELCH = 10,
    PARZEN = 11,
    EXPONENTIAL = 12,
    RIEMANN = 13,
    GAUSSIAN = 14,
    TUKEY = 15,
};

template <typename... Args>
using Callback = fu2::unique_function<void(ErrorCode, Args...)>;

template <typename Clock, typename Duration>
static inline Timestamp
timepoint_to_timestamp(const std::chrono::time_point<Clock, Duration>& t)
{
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch());

    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
        t.time_since_epoch() - seconds);

    return Timestamp{.seconds = static_cast<uint64_t>(seconds.count()),
                     .nanos = static_cast<uint32_t>(nanos.count())};
}

static inline Timestamp now()
{
    return timepoint_to_timestamp(std::chrono::system_clock::now());
}

} // namespace core

#endif
