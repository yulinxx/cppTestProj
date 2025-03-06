//  C++11中的unique_lock使用起来要比lock_guard更灵活，但是效率会第一点，内存的占用也会大一点。
//  同样，unique_lock也是一个类模板，但是比起lock_guard，它有自己的成员函数来更加灵活进行锁的操作。

// 使用方式和lock_guard一样，不同的是unique_lock有不一样的参数和成员函数。它的定义是这样的：
// std::unique_lock<std::mutex> munique(mlock);

// 这样定义的话和lock_guard没有什么区别，最终也是通过析构函数来unlock。

// std::adopt_lock
// unique_lock也可以加std::adopt_lock参数，表示互斥量已经被lock，不需要再重复lock。该互斥量之前必须已经lock，提前加锁，才可以使用该参数。
// (adopt 收养；领养；采纳；采取；接受)

// std::try_to_lock
// 可以避免一些不必要的等待，会判断当前mutex能否被lock，如果不能被lock，可以先去执行其他代码。
// 这个和adopt不同，不需要自己提前加锁。举个例子来说就是如果有一个线程被lock，而且执行时间很长，那么另一个线程一般会被阻塞在那里，反而会造成时间的浪费。
// 那么使用了try_to_lock后，如果被锁住了，它不会在那里阻塞等待，它可以先去执行其他没有被锁的代码。

// std::defer_lock
// 这个参数表示暂时先不lock，之后手动去lock，但是使用之前也是不允许去lock。一般用来搭配unique_lock的成员函数去使用。
// 当使用了defer_lock参数时，在创建了unique_lock的对象时就不会自动加锁，那么就需要借助lock这个成员函数来进行手动加锁，当然也有unlock来手动解锁。这个和mutex的lock和unlock使用方法一样
// (defer 推迟；延缓；展期)

// 还有一个成员函数是try_lock，和try_to_lock参数的作用差不多，判断当前是否能lock，如果不能，先去执行其他的代码并返回false，如果可以，进行加锁并返回true
// 对于 unique_lock 的对象来说，一个对象只能和一个mutex锁唯一对应，不能存在一对多或者多对一的情况，不然会造成死锁的出现。
// 所以如果想要传递两个unique_lock对象对mutex的权限，需要运用到移动语义或者移动构造函数两种方法。
// 移动语义：

// std::unique_lock<std::mutex> munique1(mlock);
// std::unique_lock<std::mutex> munique2(std::move(munique1));
// // 此时munique1失去mlock的权限，并指向空值，munique2获取mlock的权限

// `std::unique_lock` 和 `std::lock_guard` 是 C++ 标准库中用于管理互斥锁的两个类，它们在使用方式和功能上有一些区别。

// 1. 所有权：
//    - `std::unique_lock`：`std::unique_lock` 对象可以在构造时选择是否获取互斥锁的所有权，并可以在其生命周期内多次获取和释放锁。这意味着可以灵活地控制锁的获取和释放时机。
//    - `std::lock_guard`：`std::lock_guard` 对象在构造时会立即获取互斥锁的所有权，并在其生命周期结束时自动释放锁。它无法手动释放锁，因此适用于简单的锁保护范围。

// 2. 锁的所有权转移：
//    - `std::unique_lock`：`std::unique_lock` 对象可以通过移动语义将锁的所有权从一个对象转移给另一个对象。这样可以在不同线程之间传递锁的所有权。
//    - `std::lock_guard`：`std::lock_guard` 对象没有提供所有权转移的功能。

// 3. 锁的灵活性：
//    - `std::unique_lock`：`std::unique_lock` 对象提供了更多的灵活性，例如可以选择延迟锁的获取，或者在不同的代码段中获取和释放锁。还可以通过构造函数参数指定锁的类型（独占锁或共享锁）和锁的行为（阻塞锁或非阻塞锁）。
//    - `std::lock_guard`：`std::lock_guard` 对象具有固定的锁定行为和范围，无法在代码段中手动释放锁或更改锁的行为。

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool isReady = false;

void worker1()
{
    std::cout << "Worker 1 is Running." << std::endl;

    std::lock_guard<std::mutex> lock(mtx);

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    // 设置标志为已准备好
    isReady = true;

    std::cout << "Worker 1 send notify." << std::endl;
    // 通知等待的线程
    cv.notify_one();
}

void worker2()
{
    std::cout << "Worker 2 is Running." << std::endl;

    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    if (lock.try_lock())
    {
        // 成功获取互斥锁的所有权
        std::cout << "Thread acquired lock" << std::endl;
        // 执行操作
        std::cout << "Thread released lock" << std::endl;
    }
    else
    {
        // 无法获取互斥锁的所有权
        std::cout << "~~~Thread failed to acquire lock" << std::endl;

        std::unique_lock<std::mutex> lock(mtx);
        std::cout << "Wait for acquire lock" << std::endl;
        std::cout << "Worker 2 is get mutex lock." << std::endl;
    }

    // 等待标志为已准备好
    cv.wait(lock, [] { return isReady; });

    std::cout << "Worker 2 wait Ready." << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // 标志为已准备好后执行的操作
    std::cout << "Worker 2 is continuing after synchronization." << std::endl;
}

int main()
{
    std::thread t1(worker1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread t2(worker2);

    t1.join();
    t2.join();

    return 0;
}