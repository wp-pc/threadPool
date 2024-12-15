# 基于C++11的简单线程池实现

使用C++11技术实现的简单线程池，通过向工作队列添加任务来实现高并发运行。

使用可变参数模板和bind()函数实现可以添加任意数量参数的任务。
```
template<class F, class... Args>
void enqueue(F&& f, Args&&... args) {
    auto task = std::bind(std::forward<F>(f), 
                          std::forward<Args>(args)...);

    ........

}
```
使用智能锁与条件变量防止共享变量访问冲突。



## 运行

```

mkdir build 
cd build
cmake .. && make
./threadPool
```