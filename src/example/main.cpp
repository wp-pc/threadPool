
#include "../threadPool.hpp"

int add(int a, int b) {
    int la = a, lb = b;
    for(int i=0;i<10;++i) {
        // std::cout << a + b << std::endl;
        ++la;
        ++lb;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return la + lb;
}
int main() {
    ThreadPool pool(4);

    pool.addTask([]{
        for(;;) {
            std::cout << "hello world." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        return 1;
    });

     auto ret = pool.addTask(add, 1, 2);
     std::cout << "return = " << ret.get() << std::endl;
}