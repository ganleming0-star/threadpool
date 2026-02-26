//
// Created by Administrator on 26-2-23.
//

#include "threadpool.h"


#define TASK_MAX_THRESH_HOLD 1024

ThreadPool::ThreadPool():initThreadSize(4),taskMaxThreshHold(TASK_MAX_THRESH_HOLD),taskSize(0),
poolMode(PoolMode::Mode_FIXED),isPoolRunning(false),idleThreadSize(0),threadSizeThreshHold(100),curThreadSize(0) {
}

ThreadPool::~ThreadPool() {
}

void ThreadPool::start(int initThreadSize) {
    isPoolRunning = true;
    this->initThreadSize = initThreadSize;
    curThreadSize = initThreadSize;
    for (int i = 0; i < initThreadSize; i++) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc,this,std::placeholders::_1));
        auto id = ptr->getId();
        threads.emplace(id,std::move(ptr));
    }

    for (int i = 0; i < initThreadSize; i++) {
        threads[i]->start();
        idleThreadSize++;
        curThreadSize++;

    }

}

void ThreadPool::threadFunc(int threadId) {
    auto last = std::chrono::high_resolution_clock::now();
    while (1) {
        std::shared_ptr<Task> task;
        {
            //获取锁
            std::unique_lock<std::mutex> lock(taskQueMtx);

            if (poolMode == PoolMode::Mode_CACHED) {
                if (std::cv_status::timeout == taskQueNotEmpty.wait_for(lock,std::chrono::seconds(1))) {
                    auto now = std::chrono::high_resolution_clock::now();
                    auto dur = std::chrono::duration_cast<std::chrono::seconds>(now-last);
                    if (dur.count() > 60&& curThreadSize > initThreadSize) {
                        //回收线程
                        threads.erase(threadId);
                        curThreadSize--;
                        idleThreadSize--;
                        std::cout<<"end"<<std::endl;
                        return;
                    }
                }
            }else {
                //等待notEmpty条件
                taskQueNotEmpty.wait(lock,[&]()->bool{ return taskSize > 0;});
            }

            idleThreadSize--;


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
            task->exe();
        }
        idleThreadSize++;
        auto end = std::chrono::high_resolution_clock::now();

    }

}

void ThreadPool::setMode(PoolMode mode) {
    if (checkRunningState()) {
        return;
    }
    poolMode = mode;
}

void ThreadPool::setTaskMaxThreshHold(int max) {
    if (checkRunningState()) {
        return;
    }
    taskMaxThreshHold = max;
}

void ThreadPool::setThreadSizeThreshHold(int max) {
    if (checkRunningState()) {
        return;
    }
    if (poolMode == PoolMode::Mode_CACHED)threadSizeThreshHold = max;
}


Result ThreadPool::submitTask(std::shared_ptr<Task> sp) {
    //获取互斥锁
    std::unique_lock<std::mutex> lock(taskQueMtx);
    //等待任务队列非满
    if (!taskQueNotFull.wait_for(lock,
        std::chrono::seconds(1),[&](){
        return taskQue.size() < taskMaxThreshHold;
    })) {
        //添加任务失败
        std::cerr<<"Adding task fails"<<std::endl;
        return Result(sp,false);
    }
    //添加任务
    taskQue.emplace(sp);
    taskSize++;
    //唤醒
    taskQueNotEmpty.notify_all();

    //cached模式检测
    if (poolMode == PoolMode::Mode_CACHED&&taskSize > idleThreadSize&&curThreadSize<threadSizeThreshHold) {
        //创建新线程
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc,this,std::placeholders::_1));
        auto id = ptr->getId();
        threads.emplace(id,std::move(ptr));
        //启动
        threads[id]->start();
        curThreadSize++;
        idleThreadSize++;
    }
    //返回结果
    return Result(sp);
}

bool ThreadPool::checkRunningState() const {
    return isPoolRunning;
}


//////////////////////
int Thread::generateId = 0;

Thread::Thread(ThreadFunc new_func):func(std::move(new_func)),threadId(generateId++) {
}

Thread::~Thread() {
}

int Thread::getId() const {
    return threadId;
}

//启动线程
void Thread::start() {
    std::thread t(func,threadId);
    t.detach();//线程分离(t出作用域后会删除)
}


Result::Result(std::shared_ptr<Task> task, bool isValid):task_(task),isValid_(isValid) {
    task->setResult(this);
}

Any Result::get() {
    if (!isValid_) {
        return "";
    }
    sem_.wait();//任务执行完才会继续。否则阻塞用户线程
    return std::move(any_);
}

void Result::setVal(Any any) {
    this->any_ = std::move(any);
    sem_.post();
}


void Task::exe() {
    result_->setVal(run());
}

void Task::setResult(Result *res) {
    result_ = res;
}

Task::Task() : result_(nullptr) {
}

