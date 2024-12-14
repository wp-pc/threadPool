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

    // add task for work_queue
    template <class F, class... Args>
    inline void enqueue(F &&f, Args &&...args)
    {
        std::cout << "push a task.\n";
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        std::unique_lock<std::mutex> lock{m_queueMutex};
        if (m_stop)
        {
            throw std::runtime_error("the threadPool is stopped.");
            // throw std::runtime_error("the threadPool is stopped. ");
        }
        m_tasks.emplace(task);
        lock.unlock();
        m_cv.notify_one();
    }

    // m_stop the thread_pool
    void stop();
    ~ThreadPool();

private:
    std::vector<std::thread> m_workers;
    std::queue<Task> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_cv;
    std::thread m_mangerThead;
    bool m_stop;
    
private:
    void executeManger();
};

#endif