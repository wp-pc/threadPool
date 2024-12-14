#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP
#include <vector>
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
 
class ThreadPool {
public:
    ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i < threads; ++i) {
            workerThreads.emplace_back(
                [this] {
                    while(true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queueMutex);
                            this->condition.wait(lock,
                                [this]{ return this->stop || !this->tasks.empty(); });
                            if(this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        
                        task();
                    }
                }
            );
        }
    }
 
    template<class F, class... Args>
    void enqueue(F&& f, Args&&... args) {
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if(stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
 
            tasks.emplace(task);
        }
        condition.notify_one();
    }
 
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker : workerThreads)
            worker.join();
    }
 
private:
    std::vector<std::thread> workerThreads;
    std::queue<std::function<void()>> tasks;
 
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};
#endif