// 实现第三个线程等待前两个线程完成后再执行，使用条件变量（`std::condition_variable`）和计数器来实现线程同步。
// 下面是一个示例代码，演示了如何让第三个线程等待前两个线程完成：

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
int count = 0;

int testNum = 0;

bool bStop = false;

void thread1()
{
    std::cout << "thread 1: Init." << std::endl;

    int i = 0;
    while (i < 10)
    {
        i++;
        // 线程1的工作
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        // 完成工作后增加计数器
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "thread 1: " << testNum++ << std::endl;
        }

        cv.notify_one(); // 通知等待的线程
    }

    count++;
    bStop = true;
}

void thread2()
{
    std::cout << "thread 2: Init." << std::endl;

    while (!bStop)
    {
        // 线程2的工作
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        // 完成工作后增加计数器
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "thread 2: " << testNum++ << std::endl << std::endl;
        }

        cv.notify_one(); // 通知等待的线程
    }

    count++;
}

void thread3()
{
    std::unique_lock<std::mutex> lock(mtx);

    // 等待前两个线程完成
    cv.wait(lock, [] { return count == 2; });

    // 前两个线程完成后执行的工作
    std::cout << "Thread 3: Waited for thread 1 and thread 2 to finish. " << testNum << std::endl << std::endl;
}

int main()
{
    std::thread t1(thread1);
    std::thread t2(thread2);
    std::thread t3(thread3);

    t1.join();
    t2.join();
    t3.join();

    return 0;
}