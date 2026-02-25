#include "threadpool.h"
#include <chrono>

class TestTask: public Task {
public:
    TestTask(int begin, int end) {
        this->begin = begin;
        this->end = end;
    }
    Any run() override {
        long long sum = 0;
        for (int i = begin; i <= end; i++) {
            sum+= i;
        }
        return sum;
    }

private:
    int begin;
    int end;
};
int main() {
    ThreadPool pool;
    pool.start(4);


    Result res1 = pool.submitTask(std::make_shared<TestTask>(1,10000));
    Result res2 =pool.submitTask(std::make_shared<TestTask>(10001,20000));
    Result res3 =pool.submitTask(std::make_shared<TestTask>(20001,30000));

    long long sum = res1.get().cast_<long long>() + res2.get().cast_<long long>() + res3.get().cast_<long long>();

    std::cout<<sum<<std::endl;



    getchar();
    return 0;
}