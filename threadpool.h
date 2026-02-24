

#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>
#include <iostream>
//任务基类
class Task {
public:
    virtual ~Task() = default;

    virtual void run() = 0;
};

//线程池模式
enum class PoolMode {
    Mode_FIXED,
    Mode_CACHED
};
//线程
class Thread {
public:
    //线程函数类型
    using ThreadFunc = std::function<void()>;
    Thread(ThreadFunc new_func);
    ~Thread();
    //启动线程
    void start();
private:
    ThreadFunc func;
};
//线程池类型
class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    //启动线程池
    void start(int initThreadSize = 4);
    //设置模式
    void setMode(PoolMode mode);
    //设置任务队列上线
    void setTaskMaxThreshHold(int  max);
    //提交任务
    void submitTask(std::shared_ptr<Task> sp);
private:
    void threadFunc();

private:
    std::vector<Thread*> threads;//线程列表
    int initThreadSize;//初始线程数
    std::queue<std::shared_ptr<Task>> taskQue;//任务队列
    std::atomic_int taskSize;//任务数量
    int taskMaxThreshHold; //任务数量上线

    std::mutex taskQueMtx;//任务队列互斥锁
    std::condition_variable taskQueNotFull;//任务队列非满条件变量
    std::condition_variable taskQueNotEmpty;//任务队列非空条件变量
    PoolMode poolMode;//线程池模式
};



#endif //THREADPOOL_H
