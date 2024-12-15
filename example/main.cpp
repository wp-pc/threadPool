
#include "../threadPool.hpp"


int add(int a, int b) {
    for(;;) {
        std::cout << a + b << std::endl;
        ++a;
        ++b;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return a + b;
}
int main() {
    ThreadPool pool(4);

    pool.enqueue([]{
        for(;;) {
            std::cout << "hello world." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        return 1;
    });

    pool.enqueue(add, 1, 2);
}