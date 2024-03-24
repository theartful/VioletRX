#include <boost/asio.hpp>
#include <gflags/gflags.h>
#include <spdlog/spdlog.h>

#include "async_core/async_receiver.h"
#include "server.h"

DEFINE_string(url, "0.0.0.0:50050", "Server URL");

int main(int argc, char** argv)
{
    // Parse command line flags.
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    // Start the server.
    violetrx::AsyncReceiver::sptr receiver = violetrx::AsyncReceiver::make();
    violetrx::GrpcServer server{receiver, FLAGS_url};

    // Wait for SIGTERM/SIGINT signals.
    boost::asio::io_context ctx;
    boost::asio::signal_set signals{ctx, SIGINT, SIGTERM};

    signals.async_wait(
        [&]([[maybe_unused]] const boost::system::error_code& error,
            [[maybe_unused]] int signal_number) {
            spdlog::info("Shutting down...");
            server.Shutdown();
        });

    ctx.run();
}
