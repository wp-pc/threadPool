#include "threadPool.h"

#include <iostream>

ThreadPool::ThreadPool(size_t threads) 
    :stop(false)
{
    for(int i=0;i < threads; ++i) {
        m_workers.emplace_back([this](){
            while(true) {
                std::unique_lock<std::mutex> lock{m_queueMutex};

                std::cout << "in worker constor\n";
                m_cv.wait(lock, [this](){
                    if(stop || !(m_tasks.empty()))
                        return true;
                    else
                        return false; 
                });

                if(stop && m_tasks.empty())
                    return ;

                auto task = std::move(m_tasks.front());
                m_tasks.pop();
                lock.unlock();
                task();
            }
        });
    }
}

// push a task to taskQueue
// template<class F, class... Args>
// void ThreadPool::enqueue(F&& f, Args&&... args) {
//     std::cout << "push a task.\n";
//     auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
//     std::unique_lock<std::mutex> lock{m_queueMutex};
//     if(stop) {
//         throw std::runtime_error("the threadPool is stopped. ");
//     }
//     m_tasks.emplace(task);
//     lock.unlock();
//     m_cv.notify_one();
// }


ThreadPool::~ThreadPool() {
    std::unique_lock<std::mutex> lock(m_queueMutex);
    stop = true;
    lock.unlock();
    for(auto& t : m_workers) {
        if(t.joinable())
            t.join();
    }
}