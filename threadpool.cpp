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

    for (;;) {
        std::shared_ptr<Task> task;
        {
            //获取锁
            std::unique_lock<std::mutex> lock(taskQueMtx);
            //等待notEmpty条件
            taskQueNotEmpty.wait(lock,[&]()->bool{ return taskSize > 0;});
            //取任务
            task = taskQue.front();
            taskQue.pop();
            taskSize--;
            //唤醒
            if (!taskQue.empty()) {
                taskQueNotEmpty.notify_all();
            }

            taskQueNotFull.notify_all();
        }

        //执行任务
        if ( task) {
            task->run();
        }

    }

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
    if (!taskQueNotFull.wait_for(lock,
        std::chrono::seconds(1),[&](){
        return taskQue.size() < taskMaxThreshHold;
    })) {
        //添加任务失败
        std::cerr<<"Adding task fails"<<std::endl;
        return;
    }
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
