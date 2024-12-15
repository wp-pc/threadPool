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
    void enqueue(F&& f, Args&& ...args);
    void stop();
    ~ThreadPool();

private:
    std::vector<std::thread> m_workers;
    std::queue<Task> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_cv;

    bool m_stop;
private:
    void executeTask();
};


ThreadPool::ThreadPool(size_t threads)
    : m_stop(false)
{
    for (int i = 0; i < threads; ++i)
    {
        m_workers.emplace_back(&ThreadPool::executeTask,this);
    }
}

// add task for work_queue
template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&& ...args) -> void // std::future<typename std::result_of<F(Args...)>::type>
{
    auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

    std::unique_lock<std::mutex> lock{m_queueMutex};
    if (m_stop)
    {
        throw std::runtime_error("the threadPool is stopped.");
    }
    m_tasks.emplace(task);
    lock.unlock();
    m_cv.notify_one();
}

// m_stop the thread_pool
inline void ThreadPool::stop()
{
    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_stop = true;
    lock.unlock();
    for (auto &t : m_workers)
    {
        if (t.joinable())
            t.join();
    }
}

// exectuTask
void ThreadPool::executeTask()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock{m_queueMutex};
        m_cv.wait(lock,[this]{
            return m_stop || !(m_tasks.empty());
        });
        if (m_stop && m_tasks.empty())
            return;

        auto task = std::move(m_tasks.front());
        m_tasks.pop();

        lock.unlock();
        task();
    }
}

ThreadPool::~ThreadPool()
{
    stop();
}

#endif