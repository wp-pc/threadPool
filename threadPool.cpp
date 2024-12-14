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


void ThreadPool::enqueue(Task&& task) {

    std::cout << "hello world.\n";
    std::unique_lock<std::mutex> lock{m_queueMutex};
    m_cv.wait(lock, [this](){
        return stop || !(m_tasks.empty());
    });

    m_tasks.emplace(task);

    lock.unlock();
    m_cv.notify_one();

}




ThreadPool::~ThreadPool() {
    stop = true;
}