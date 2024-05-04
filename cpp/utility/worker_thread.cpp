#include "worker_thread.h"

#include <atomic>
#include <chrono>
#include <fmt/chrono.h>
#include <spdlog/spdlog.h>

namespace violetrx
{

static constexpr uint64_t FACTOR = (1 << 16);
static constexpr uint64_t ALPHA = FACTOR / 16;
static constexpr uint64_t ONE_MINUS_ALPHA = (FACTOR - ALPHA);

// TODO: make it configurable
static constexpr auto MAX_TASK_DURATION = std::chrono::seconds(2);

WorkerThread::WorkerThread() : avgLatency{0}, inTask{false} {}

WorkerThread::~WorkerThread()
{
    spdlog::debug("~WorkerThread");

    if (isJoinable()) {
        stop();
        join();
    }

    // Print all enqueued tasks for debugging
    fmt::memory_buffer buffer;
    {
        int count = 0;
        Task task;
        while (tasks.try_dequeue(task)) {
            if (count == 0) {
                spdlog::debug("\tUnfinished Tasks:");
            }
            count++;

            spdlog::debug("\t\t{}:\t{}", count, task.cmd);
        }
    }
}

void WorkerThread::stop() { thread.request_stop(); }
void WorkerThread::join() { thread.join(); }

bool WorkerThread::scheduleImpl(const char* cmd, Function<void()> task)
{
    if (!isPaused()) {
        scheduleForcedImpl(cmd, std::move(task));
        return true;
    } else {
        return false;
    }
}

bool WorkerThread::isPaused()
{
    if (inTask.load(std::memory_order_relaxed) &&
        (std::chrono::steady_clock::now() - taskStartTime) >
            MAX_TASK_DURATION) {
        spdlog::error("The worker thread has been doing one task ({}) for over "
                      "{} seconds. The thread will not accept any more tasks "
                      "until the task queue is empty to prevent overflowing!",
                      lastCmd.load(), MAX_TASK_DURATION.count());
        return true;
    }

    return false;
}

void WorkerThread::scheduleForcedImpl(const char* cmd, Function<void()> task)
{
    tasks.enqueue(Task{std::move(task), std::chrono::steady_clock::now(), cmd});
}

void WorkerThread::start()
{
    if (isJoinable()) {
        spdlog::error("Worker thread is already running!");
        return;
    }

    thread = std::jthread(std::bind_front(&WorkerThread::startEventLoop, this));
}

bool WorkerThread::isJoinable() const { return thread.joinable(); }

std::chrono::microseconds WorkerThread::getAvgLatency()
{
    return std::chrono::microseconds(avgLatency / FACTOR);
}

void WorkerThread::updateAvgLatency(const Task& task)
{
    // doing fixed point arithmetic instead of floating point arithmetic
    uint64_t latency = std::chrono::duration_cast<std::chrono::microseconds>(
                           std::chrono::steady_clock::now() - task.timestamp)
                           .count() *
                       FACTOR;

    // exponentially moving average
    avgLatency = (avgLatency * ONE_MINUS_ALPHA + latency * ALPHA) / FACTOR;
}

void WorkerThread::startEventLoop(std::stop_token stopToken)
{
    Task task;

    while (!stopToken.stop_requested()) {
        if (tasks.wait_dequeue_timed(task, std::chrono::seconds(1))) {
            updateAvgLatency(task);

            taskStartTime = std::chrono::steady_clock::now();
            lastCmd = task.cmd;

            inTask = true;

            try {
                task.func();
            } catch (const std::exception& e) {
                spdlog::error("Task ({}) raised an exception: {}",
                              lastCmd.load(), e.what());
            }
            inTask = false;
        }
    }
}

std::thread::id WorkerThread::getId() const { return thread.get_id(); }

} // namespace violetrx
