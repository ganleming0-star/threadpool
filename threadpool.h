
#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <unordered_map>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>
#include <iostream>
#include <map>

class Task;
//任意类型
class Any {
public:
    Any() = default;
    ~Any() = default;
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;
    Any (Any&&) = default;
    Any& operator=(Any&&) = default;
    template<class T>
    Any(T data) : base_(new Derive<T>(data)){}

    //提取data数据
    template<class T>
    T cast_() {
        Derive<T>*  pd = dynamic_cast<Derive<T>*>(base_.get());
        if (pd == nullptr) {
            throw "type is not match";
        }
        return pd->data_;
    }
private:
    class Base {
    public:
        virtual ~Base() = default;
    };

    template<class T>
    class Derive : public Base {
    public:
        Derive(T data): data_(data){}

        T data_;
    };

private:
    std::unique_ptr<Base> base_;
};

//信号量
class Semaphore {
public:
    Semaphore(int limit = 0) : resLimit_(limit){

    }
    ~Semaphore() = default;


    void wait() {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock,[this](){
            return resLimit_ > 0;
        });
        resLimit_--;
    }

    void post() {
        std::unique_lock<std::mutex> lock(mtx_);
        resLimit_++;
        cond_.notify_all();
    }


private:
    int resLimit_;
    std::mutex mtx_;
    std::condition_variable cond_;

};
//接受提交到线程池的task任务执行完后的返回值类型Result
class Result {
public:
    Result (std::shared_ptr<Task> task,bool isValid = true);
    ~Result() = default;
    Any get() ;
    void setVal(Any any);




private:
    Any any_;//存储任务的返回值
    Semaphore sem_;//线程通信信号量
    std::shared_ptr<Task> task_;//指向任务对象
    std::atomic_bool isValid_;//返回值是否有效

};
//任务基类
class Task {
public:
    Task();
    virtual ~Task() = default;
    void exe();
    void setResult(Result *res);
    virtual Any run() = 0;

private:
    Result* result_;
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
    using ThreadFunc = std::function<void(int)>;
    Thread(ThreadFunc new_func);
    ~Thread();
    //启动线程
    void start();
    //获取线程ID
    int getId() const;
private:
    ThreadFunc func;
    static  int generateId;
    int threadId;
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
    void setThreadSizeThreshHold(int max);
    //提交任务
    Result submitTask(std::shared_ptr<Task> sp);
private:
    void threadFunc(int threadId);
    bool checkRunningState()const;

private:
    //std::vector<std::unique_ptr<Thread>> threads;//线程列表
    std::unordered_map<int,std::unique_ptr<Thread>> threads;
    int initThreadSize;//初始线程数
    std::atomic_int curThreadSize;//记录当前线程池里面的线程数
    std::atomic_int idleThreadSize;//线程池中空闲线程数量
    int threadSizeThreshHold;//线程数量上限

    std::queue<std::shared_ptr<Task>> taskQue;//任务队列
    std::atomic_int taskSize;//任务数量
    int taskMaxThreshHold; //任务数量上线

    std::mutex taskQueMtx;//任务队列互斥锁
    std::condition_variable taskQueNotFull;//任务队列非满条件变量
    std::condition_variable taskQueNotEmpty;//任务队列非空条件变量
    std::condition_variable exitCond;//线程池关闭条件变量
    PoolMode poolMode;//线程池模式
    std::atomic_bool isPoolRunning;//线程池运行状态

};







#endif //THREADPOOL_H
