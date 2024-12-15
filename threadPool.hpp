#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <iostream>
#include <future>
class ThreadPool
{
    using Task = std::function<void()>;

public:
    explicit ThreadPool(size_t threads);

    // add task for work_queue
    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args)->void;
        // std::future<typename std::result_of<F(Args...)>::type>;

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

template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args)->void
    // std::future<typename std::result_of<F(Args...)>::type>
{
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

ThreadPool::ThreadPool(size_t threads)
    : m_stop(false)
{

    // 初始化管理线程
    m_mangerThead = std::thread([this]
                                {
        for(int i=0;i<20;++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            stop();
        } });
    m_mangerThead.detach();
    for (int i = 0; i < threads; ++i)
    {
        m_workers.emplace_back([this]()
                               {
            while(true) {
                std::unique_lock<std::mutex> lock{m_queueMutex};
                m_cv.wait(lock, [this](){
                    if(m_stop || !(m_tasks.empty()))
                        return true;
                    else
                        return false; 
                });

                if(m_stop && m_tasks.empty())
                    return ;

                auto task = std::move(m_tasks.front());
                m_tasks.pop();
                lock.unlock();
                task();
            } });
    }
}

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

ThreadPool::~ThreadPool()
{
    stop();
}
#endif