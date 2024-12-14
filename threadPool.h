#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>

class ThreadPool
{
    using Task = std::function<void()>;
public:
    explicit ThreadPool(size_t threads);
    void enqueue(Task&& task);
    ~ThreadPool();
private:
    std::vector<std::thread> m_workers;
    std::queue<Task> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_cv;
    bool stop;
};

#endif