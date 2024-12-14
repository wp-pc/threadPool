#include "threadPool.h"

#include <iostream>

ThreadPool::ThreadPool(size_t threads) 
    :m_stop(false)
{

    // 初始化管理线程
    m_mangerThead = std::thread([this]{
        for(int i=0;i<20;++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            stop();
        }
    });
    m_mangerThead.detach();
    for(int i=0;i < threads; ++i) {
        m_workers.emplace_back([this](){
            while(true) {
                std::unique_lock<std::mutex> lock{m_queueMutex};

                std::cout << "in worker constor\n";
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
//     if(m_stop) {
//         throw std::runtime_error("the threadPool is stopped. ");
//     }
//     m_tasks.emplace(task);
//     lock.unlock();
//     m_cv.notify_one();
// }


void ThreadPool::stop() {
    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_stop = true;
    lock.unlock();
    for(auto& t : m_workers) {
        if(t.joinable())
            t.join();
    }
}

ThreadPool::~ThreadPool() {
    stop();
}