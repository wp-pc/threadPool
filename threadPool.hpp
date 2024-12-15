#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <future>
#include <memory>
#include <atomic>
#include <iostream>
class ThreadPool
{
    using Task = std::function<void()>;

public:
    explicit ThreadPool(size_t threads);

    template <class F, class... Args>
    auto addTask(F&& f, Args&& ...args)->
        std::future<std::result_of_t<F(Args...)>>; // 类型后置，获取返回值

    void stop();
    ~ThreadPool();

private:

    int MAX_THREADS = 8; // 最大线程数
    bool _stop; // 是否终止

    std::thread _mangerThread; // 管理线程
    std::vector<std::thread*> _workers; // 工作线程列表
    std::queue<Task> _taskQue; // 任务队列
    std::mutex _queueMutex;
    std::condition_variable _cv;
    std::atomic<int> _runningTasks;

private:
    void executeTask();   // 执行任务
    void executeManger(); // 线程管理函数
};


// ch: 构造函数，创建线程池
// en: create the thread_pool
ThreadPool::ThreadPool(size_t threads)
    : _stop(false)
{
    _mangerThread = std::thread(&ThreadPool::executeManger, this);
    for (int i = 0; i < threads; ++i)
    {
        _workers.emplace_back(new std::thread(&ThreadPool::executeTask, this));
    }
}

// ch: 添加任务到工作队列
// en: add task to tasksQueue
template <class F, class... Args>
auto ThreadPool::addTask(F&& f, Args&& ...args) ->
    std::future<std::result_of_t<F(Args...)>>
{
    using RetType = decltype(f(args...));

    auto task = std::make_shared<std::packaged_task<RetType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<RetType> ret_future = task->get_future();

    std::unique_lock<std::mutex> lock{_queueMutex};    
    if (_stop)
    {
        throw std::runtime_error("the threadPool is stopped.");
    }
    // zh: 添加任务到任务队列
    // en: add task to tasksQueue
    _taskQue.emplace([task] {
        (*task)();
    });
    lock.unlock();
    _cv.notify_one();

    return ret_future;
}

// ch: 停止线程池
// en: stop the threadPool
inline void ThreadPool::stop()
{
    std::unique_lock<std::mutex> lock(_queueMutex);
    _stop = true;
    if (_mangerThread.joinable())
        _mangerThread.join();

    lock.unlock();
    for (size_t i = 0; i < _workers.size(); ++i) {
        if (_workers[i]->joinable()) {
            _workers[i]->join();
        }
        delete _workers[i];
        _workers[i] = nullptr;
    }
    _workers.clear();
}

// ch: 执行单个任务
// en: execute single task
void ThreadPool::executeTask()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock{_queueMutex};
        _cv.wait(lock,[this]{
            return _stop || !(_taskQue.empty());
        });
        if (_stop && _taskQue.empty())
            return;
        _runningTasks++;
        auto task = std::move(_taskQue.front());
        _taskQue.pop();

        lock.unlock();
        task();
        _runningTasks--;
    }
}

// zh: 线程管理函数
// en: thread manager
void ThreadPool::executeManger() {
    while (!_stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // 3s 检查一次
        std::cout << "running tasks: " << _runningTasks << std::endl;
        std::cout << "live tasks: " << _taskQue.size() << std::endl;
        std::cout << "sum threads: " << _workers.size() << std::endl;
        // 如果任务数大于线程数，则增加线程
        if (_runningTasks>0 && !_taskQue.empty() && _workers.size()<MAX_THREADS) {
            std::unique_lock<std::mutex> lock{_queueMutex};
            _workers.emplace_back(new std::thread(&ThreadPool::executeTask, this));
        }
    }
}

// ch: 析构函数，停止线程池
// en: stop the threadPool
ThreadPool::~ThreadPool()
{
    stop();
}

#endif