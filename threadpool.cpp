//
// Created by Administrator on 26-2-23.
//

#include "threadpool.h"

#define TASK_MAX_THRESH_HOLD 1024

ThreadPool::ThreadPool():initThreadSize(4),taskMaxThreshHold(TASK_MAX_THRESH_HOLD),taskSize(0),poolMode(PoolMode::Mode_FIXED) {
}

ThreadPool::~ThreadPool() {
}

void ThreadPool::start(int initThreadSize) {
    this->initThreadSize = initThreadSize;
    for (int i = 0; i < initThreadSize; i++) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc,this));
        threads.emplace_back(std::move(ptr));
    }

    for (auto& m_thread : threads) {
        m_thread->start();
    }

}

void ThreadPool::threadFunc() {
    std::cout<<std::this_thread::get_id()<<" start"<<std::endl;

}

void ThreadPool::setMode(PoolMode mode) {
    poolMode = mode;
}

void ThreadPool::setTaskMaxThreshHold(int max) {
    taskMaxThreshHold = max;
}



void ThreadPool::submitTask(std::shared_ptr<Task> sp) {
    //获取互斥锁
    std::unique_lock<std::mutex> lock(taskQueMtx);
    //等待任务队列非满
    taskQueNotFull.wait(lock,[&](){
        return taskQue.size() < taskMaxThreshHold;
    });
    //添加任务
    taskQue.emplace(sp);
    taskSize++;
    //唤醒
    taskQueNotEmpty.notify_all();
}



//////////////////////

//启动线程
Thread::Thread(ThreadFunc new_func):func(new_func) {
}

Thread::~Thread() {
}

void Thread::start() {
    std::thread t(func);
    t.detach();//线程分离(t出作用域后会删除)
}
