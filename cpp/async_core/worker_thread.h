#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H

#include <atomic>
#include <blockingconcurrentqueue.h>
#include <chrono>
#include <function2/function2.hpp>
#include <functional>
#include <thread>

namespace core
{
class WorkerThread
{
    template <typename... Signature>
    using Function = fu2::unique_function<Signature...>;

private:
    struct Task {
        Function<void()> func;
        std::chrono::steady_clock::time_point timestamp;
        const char* cmd;
    };

public:
    using sptr = std::shared_ptr<WorkerThread>;

    WorkerThread();
    ~WorkerThread();

    void stop();
    void join();
    void start();
    std::thread::id getId();

    // NOTE: this function will synchronously execute the task if we're already
    // in the worker thread
    template <typename Function, typename... Args>
    bool schedule(const char* cmd, Function&& f, Args&&... args)
    {
        if (std::this_thread::get_id() == getId()) {
            std::invoke(f, std::forward<Args>(args)...);
            return true;
        } else {
            return scheduleImpl(cmd,
                                std::bind_front(std::forward<Function>(f),
                                                std::forward<Args>(args)...));
        }
    }

    // NOTE: this function will synchronously execute the task if we're already
    // in the worker thread
    template <typename Function, typename... Args>
    void scheduleForced(const char* cmd, Function&& f, Args&&... args)
    {
        if (std::this_thread::get_id() == getId()) {
            std::invoke(f, std::forward<Args>(args)...);
        } else {
            scheduleForcedImpl(cmd,
                               std::bind_front(std::forward<Function>(f),
                                               std::forward<Args>(args)...));
        }
    }

    bool isPaused();

private:
    bool scheduleImpl(const char* cmd, Function<void()> task);
    void scheduleForcedImpl(const char* cmd, Function<void()> task);
    void updateAvgLatency(const Task& task);
    std::chrono::microseconds getAvgLatency();

    void startEventLoop(std::stop_token);

private:
    moodycamel::BlockingConcurrentQueue<Task> tasks;
    uint64_t avgLatency;

    std::atomic<bool> inTask;
    std::chrono::steady_clock::time_point taskStartTime;

    std::jthread thread;
    std::atomic<const char*> lastCmd;
};
} // namespace core

#endif // WORKER_THREAD_H
