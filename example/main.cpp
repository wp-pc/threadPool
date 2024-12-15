#include <iostream>
#include <ctime>
#include <type_traits>

#include "../threadPool.hpp"

void fun(int a) {
    while(1) {
        std::cout << "Thread id " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

class A {
public:
    A() = default;
    void operator()(){
        for(int i=0;i<10;++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::cout << "Thread id: " << std::this_thread::get_id() << std::endl;
        }
    }
};

// void fun() {

// }
int main() {


    std::cout << "Starting the proess. " << std::endl;
    std::cout << typeid(int).name() << std::endl;
    ThreadPool pool(4);

    pool.enqueue([]() {
        while(1) {
            std::cout << "Thread id: " << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    pool.enqueue([]() {
        while(1) {
            std::cout << "Thread id: " << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    pool.enqueue(fun, 3);
    pool.enqueue(A());
}

