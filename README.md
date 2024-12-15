# 基于C++11的线程池实现

使用C++11技术实现的简单线程池，通过向工作队列添加任务来实现高并发运行。

使用可变参数模板和bind()函数实现可以添加任意数量参数的任务。
```C++
template<class F, class... Args>
void enqueue(F&& f, Args&&... args) {
    auto task = std::bind(std::forward<F>(f), 
                          std::forward<Args>(args)...);

    ........

}
```
使用智能锁与条件变量防止共享变量访问冲突。



## 运行

```shell

mkdir build 
cd build
cmake .. && make
./threadPool
```



## 设计思路

**tasks**: 任务队列，每当有新任务时，就addTask到该队列

**workers**: 工作线程，不断地从tasks中取任务执行

**queueMutex**: 任务队列互斥锁，防止在addTask时出现冲突

**condition_variable**: 条件变量，当任务队列为空时阻塞线程，等待任务被添加进队列

**function<void()>** : 函数对象，tasks队列的成员，当前每一个都可以当成返回值为void、无参数的函数执行，由于后续添加任务时多数是带有返回值和参数的，因此需要使用bind函数绑定所有参数适配成void()类型，使用future获取所添加的任务返回值

# 实现步骤

### 整体结构

```C++
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
    std::vector<std::thread> _workers;
    std::queue<Task> _tasks;
    std::mutex _queueMutex;
    std::condition_variable _cv;

    bool _stop;
private:
    void executeTask();
};
```

### 初始化

创建指定数量的线程，每个线程负责执行excuteTask函数(不断取任务执行)

```C++
ThreadPool::ThreadPool(size_t threads)
    : _stop(false)
{
    for (int i = 0; i < threads; ++i)
    {
        _workers.emplace_back(&ThreadPool::executeTask,this);
    }
}
```

### 执行任务

不断从tasks队列取任务并执行，由于同时会有多个线程读写tasks队列会出现冲突，因为需要加锁，使用条件变量，当tasks队列为空时阻塞线程，如果线程池停止(stop=true)同时队列为空则整个线程终止.



```C++
void ThreadPool::executeTask()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock{_queueMutex};
        _cv.wait(lock,[this]{
            return _stop || !(_tasks.empty());
        });
        if (_stop && _tasks.empty())
            return;

        auto task = std::move(_tasks.front());
        _tasks.pop();

        lock.unlock();
        task();
    }
}
```

### 添加任务

**可变参数模板**

该语法支持添加任意数目参数的函数，通过执行f(args...)可执行添加的任务函数。可以使用bind函数绑定f函数和它的所有参数，将其转换成无参的task函数对象（由于fun函数形参是&&，右值引用型，故需用到forward函数，详细请搜索右值引用）

```c++
template<class F, class... Args>
ret fun(F&& f, Args&& ...args){
    std::funtion<void()> task = std::bind(std::forward<F>(f), 
                                          std::forward<Args>(args)...);
    .....
}
```

**decltype**可以根据变量推测出类型。

添加任务到tasks队列，加锁防止冲突

```C++
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
    // 添加任务到任务队列
    _tasks.emplace([task] {
        (*task)();
    });
    lock.unlock();
    _cv.notify_one();

    return ret_future;
}
```


### 停止线程池

将_stop置为true，同时将workers中所有线程执行完毕，即可停止线程池

```C++
void ThreadPool::stop()
{
    std::unique_lock<std::mutex> lock(_queueMutex);
    _stop = true;
    lock.unlock();
    for (auto &t : _workers)
    {
        if (t.joinable())
            t.join();
    }
}
```
