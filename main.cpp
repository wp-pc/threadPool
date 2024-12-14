#include <iostream>

#include "threadPool.h"

void fun(int a) {
    while(1) {
        std::cout << "Thread 3: a= " << a << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
int main() {

    std::cout << "Starting the proess. " << std::endl;
    ThreadPool pool(4);

    pool.enqueue([]() {
        while(1) {
            std::cout << "Thread 1" << std::endl;
            // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });
    pool.enqueue([]() {
        while(1) {
            std::cout << "Thread 2" << std::endl;
            // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });

    // pool.enqueue(fun, 3);
}