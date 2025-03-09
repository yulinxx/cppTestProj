// 解读github上流行的ThreadPool源码 - Halo_run - 博客园
// https://www.cnblogs.com/chenleideblog/p/12915534.html
// C++17版警告，添加 #if __cplusplus >= 201703L 宏进行版本区分，
// 并要在CMake中添加如下，否则  Visual Studio下编译， __cplusplus宏为 199711L
// if(MSVC)
//     target_compile_options(${LIB_NAME} PUBLIC "/Zc:__cplusplus")
// endif()

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool {
public:
    ThreadPool(size_t);

    // 将任务添加到线程池的任务队列中
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
#if __cplusplus >= 201703L
        ->std::future<typename std::invoke_result<F, Args...>::type>;
#else
        ->std::future<typename std::result_of<F(Args...)>::type>;
#endif // __cplusplus >= 201703L
    ~ThreadPool();

private:
    // need to keep track of threads so we can join them
    // 用于存放任务的队列，用queue队列进行保存.任务类型为std::function<void()>.
    // 因为 std::function是通用多态函数封装器，也就是说本质上任务队列中存放的是一个个函数
    std::vector< std::thread > workers; // 用于存放线程的数组

    // the task queue
    std::queue< std::function<void()> > tasks;

    // synchronization
    // 一个访问任务队列的互斥锁，在插入任务或者线程取出任务都需要借助互斥锁进行安全访问
    std::mutex queue_mutex;

    // 一个用于通知线程任务队列状态的条件变量，若有任务则通知线程可以执行，否则进入wait状态
    std::condition_variable condition;

    // 标识线程池的状态，初始化成员变量stop为false，即表示线程池启动着
    bool stop;
};


///////////////////////////////////////////////////////////////////////

// the constructor just launches some amount of workers
// 参数 threads 表示线程池中要创建多少个线程
inline ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i) // 依次创建threads个线程，并放入线程数组workers中
        workers.emplace_back(
            [this]
            {
                for(;;)
                {
                    // 创建一个封装void()函数的std::function对象task，用于接收后续从任务队列中弹出的真实任务.
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);   // 锁， 对任务加锁
                        // 若后续条件变量来了通知，线程就会继续往下进行
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });

                        // 若线程池已经停止且任务队列为空，则线程返回，没必要进行死循环.
                        if(this->stop && this->tasks.empty())
                            return;

                        // 将任务队列中的第一个任务用task标记，然后将任务队列中该任务弹出.
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            }
        );
}


// add new work item to the pool
// equeue是一个模板函数，其类型形参为F与Args.其中class... Args表示多个类型形参.
// auto用于自动推导出equeue的返回类型，函数的形参为(F&& f, Args&&... args)，
// 其中&&表示右值引用.表示接受一个F类型的f，与若干个Args类型的args.

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
#if __cplusplus >= 201703L
-> std::future<typename std::invoke_result<F, Args...>::type>    // 获得以Args为参数的F的函数类型的返回类型
#else
-> std::future<typename std::result_of<F(Args...)>::type>
#endif // __cplusplus >= 201703L
{
#if __cplusplus >= 201703L
    using return_type = typename std::invoke_result<F, Args...>::type;
#else
    using return_type = typename std::result_of<F(Args...)>::type;
#endif // __cplusplus >= 201703L

    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

#endif
