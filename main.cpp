#include "threadpool.h"
#include <chrono>

class TestTask: public Task {
    void run() override {
        std::cout<<std::this_thread::get_id()<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
};
int main() {
    ThreadPool pool;
    pool.start(4);
    pool.submitTask(std::make_shared<TestTask>());
    pool.submitTask(std::make_shared<TestTask>());
    pool.submitTask(std::make_shared<TestTask>());
    pool.submitTask(std::make_shared<TestTask>());
    pool.submitTask(std::make_shared<TestTask>());
    pool.submitTask(std::make_shared<TestTask>());
    pool.submitTask(std::make_shared<TestTask>());
    pool.submitTask(std::make_shared<TestTask>());
    getchar();
    return 0;
}