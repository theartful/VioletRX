#include <boost/asio.hpp>
#include <gflags/gflags.h>
#include <spdlog/spdlog.h>

#include "async_core/events_format.h" // IWYU pragma: keep
#include "grpc/client.h"

DEFINE_string(url, "0.0.0.0:50050", "Server URL");
DEFINE_bool(sync_only, false, "Receive sync events only and exit");

int main(int argc, char** argv)
{
    // Parse command line flags.
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    spdlog::set_level(spdlog::level::debug);

    bool connected_successfully = false;

    // Wait for SIGTERM/SIGINT signals.
    boost::asio::io_context ctx;
    boost::asio::signal_set signals{ctx, SIGINT, SIGTERM};
    signals.async_wait(
        [&]([[maybe_unused]] const boost::system::error_code& error,
            [[maybe_unused]] int signal_number) {
            spdlog::info("Shutting down...");
        });

    violetrx::GrpcClient client{FLAGS_url};
    client.Subscribe([&](const violetrx::Event& event) {
        spdlog::info("{}", event);
        if (std::holds_alternative<violetrx::SyncStart>(event)) {
            connected_successfully = true;
        }

        if (FLAGS_sync_only &&
            std::holds_alternative<violetrx::SyncEnd>(event)) {

            // Stops the event loop, and prevents any subsequent calls to
            // "ctx.run" from doing any work until "ctx.restart" is called.
            ctx.stop();
        }
    });

    ctx.run();

    spdlog::info("connected_successfully = {}", connected_successfully);
    return connected_successfully ? EXIT_SUCCESS : EXIT_FAILURE;
}
