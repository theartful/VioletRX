#include <condition_variable>
#include <iostream>
#include <mutex>

#include <gflags/gflags.h>
#include <spdlog/spdlog.h>

#include "grpc/client.h"

DEFINE_string(url, "0.0.0.0:50050", "Server URL");

int main()
{
    violetrx::GrpcClient client{FLAGS_url};

    std::string device =
        "file=/home/theartful/IQData/"
        "interesting,freq=100e6,rate=14.122e6,repeat=true,throttle=true";

    std::mutex mu;
    std::condition_variable cv;
    bool done = false;

    spdlog::info("Running SetInputDevice");
    client.SetInputDevice(device, [&](violetrx::ErrorCode err) {
        spdlog::info("SetInputDevice: {}", violetrx::errorMsg(err));

        std::lock_guard<std::mutex> lock(mu);
        done = true;
        cv.notify_one();
    });

    {
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&]() { return done; });
        done = false;
    }

    uint64_t vfo_handle;
    spdlog::info("Running AddVfoChannel");
    client.AddVfoChannel([&](violetrx::ErrorCode err, uint64_t handle) {
        spdlog::info("AddVfoChannel: {} {}", violetrx::errorMsg(err), handle);

        vfo_handle = handle;
        std::lock_guard<std::mutex> lock(mu);
        done = true;
        cv.notify_one();
    });

    {
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&]() { return done; });
        done = false;
    }

    spdlog::info("Running Start");
    client.Start([&](violetrx::ErrorCode err) {
        spdlog::info("Start: {}", violetrx::errorMsg(err));

        std::lock_guard<std::mutex> lock(mu);
        done = true;
        cv.notify_one();
    });

    {
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&]() { return done; });
        done = false;
    }

    spdlog::info("Running SetDemod");
    client.SetDemod(vfo_handle, violetrx::Demod::WFM_MONO,
                    [&](violetrx::ErrorCode err) {
                        spdlog::info("SetDemod: {}", violetrx::errorMsg(err));

                        std::lock_guard<std::mutex> lock(mu);
                        done = true;
                        cv.notify_one();
                    });

    {
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&]() { return done; });
        done = false;
    }

    // spdlog::info("Running Stop");
    // client.Stop([&](violetrx::ErrorCode err) {
    //     spdlog::info("Stop: {}", violetrx::errorMsg(err));

    //     std::lock_guard<std::mutex> lock(mu);
    //     done = true;
    //     cv.notify_one();
    // });

    // {
    //     std::unique_lock<std::mutex> lock(mu);
    //     cv.wait(lock, [&]() { return done; });
    //     done = false;
    // }
}
