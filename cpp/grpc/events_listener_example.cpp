#include <boost/asio.hpp>
#include <gflags/gflags.h>
#include <spdlog/spdlog.h>

#include "async_core/events_format.h" // IWYU pragma: keep
#include "grpc/client.h"

DEFINE_string(url, "0.0.0.0:50050", "Server URL");

int main()
{
    spdlog::set_level(spdlog::level::debug);

    violetrx::GrpcClient client{FLAGS_url};
    client.Subscribe([](const violetrx::Event& event) { spdlog::info(event); });

    // Wait for SIGTERM/SIGINT signals.
    boost::asio::io_context ctx;
    boost::asio::signal_set signals{ctx, SIGINT, SIGTERM};

    signals.async_wait(
        [&]([[maybe_unused]] const boost::system::error_code& error,
            [[maybe_unused]] int signal_number) {
            spdlog::info("Shutting down...");
        });

    ctx.run();
}
