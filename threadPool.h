#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <iostream>
class ThreadPool
{
    using Task = std::function<void()>;

public:
    explicit ThreadPool(size_t threads);

    template <class F, class... Args>

    // add task for work_queue
    inline void enqueue(F &&f, Args &&...args)
    {
        std::cout << "push a task.\n";
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        std::unique_lock<std::mutex> lock{m_queueMutex};
        if (stop)
        {
            throw std::runtime_error("the threadPool is stopped.");
            // throw std::runtime_error("the threadPool is stopped. ");
        }
        m_tasks.emplace(task);
        lock.unlock();
        m_cv.notify_one();
    }

    // stop the thread_pool
    // void stop();
    ~ThreadPool();

private:
    std::vector<std::thread> m_workers;
    std::queue<Task> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_cv;
    bool stop;
};

#endif