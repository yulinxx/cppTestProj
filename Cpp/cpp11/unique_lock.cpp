// #include <iostream>
// #include <thread>
// #include <mutex>

// std::mutex mtx;

// void Worker() {
//     std::unique_lock<std::mutex> lock(mtx);  // 构造 std::unique_lock 对象并锁定互斥锁

//     // 临界区操作
//     std::cout << "Worker thread: Acquired lock, performing work." << std::endl;
//     // ...

//     // 在 std::unique_lock 对象析构时自动释放锁
//     std::cout << "Worker thread: Lock released." << std::endl;
// }

// int maina()
// {
//     std::cout<<" ---- "<<std::endl;

//     std::thread t(Worker);
//     t.join();

//     std::cout<<" end "<<std::endl;
//     return 0;
// }

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool condition = false;

void Worker()
{
    std::cout << " ---- Worker" << std::endl;
    std::lock_guard<std::mutex> lock(mtx);

    // while (!condition)
    // {
    //     cv.wait(lock, []{ return condition; });  // 等待条件满足
    // }

    // 执行其他操作
    std::cout << "Worker thread: Condition is satisfied." << std::endl;
}

int main()
{
    std::cout << " ---- main" << std::endl;

    // std::unique_lock<std::mutex> lock(mtx);
    std::thread t(Worker);
    std::thread t2(Worker);
    std::thread t3(Worker);

    // std::this_thread::sleep_for(std::chrono::seconds(2));

    {
        std::lock_guard<std::mutex> lock(mtx);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        // condition = true;
    }

    // cv.notify_one();

    std::cout << " ---- main2" << std::endl;
    t.join();
    t2.join();
    t3.join();

    return 0;
}