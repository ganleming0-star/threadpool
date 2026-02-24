#include "threadpool.h"
#include <chrono>
int main() {
    ThreadPool pool;
    pool.start(6);
    std::this_thread::sleep_for(std::chrono::seconds(20));
    return 0;
}